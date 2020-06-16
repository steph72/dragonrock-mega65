#include "dungeon.h"
#include "character.h"
#include "congui.h"
#include "debug.h"
#include "dungeonLoader.h"
#include "encounter.h"
#include "globals.h"
#include "monster.h"

#include <conio.h>
#include <plus4.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------- opcodes ------------------------- */
#define OPC_NOP 0x00    /* no operation                       */
#define OPC_NSTAT 0x01  /* new status line                    */
#define OPC_DISP 0x02   /* display text                       */
#define OPC_WKEY 0x03   /* waitkey                            */
#define OPC_YESNO 0x04  /* request y or n                     */
#define OPC_IFREG 0x05  /* if register                        */
#define OPC_IFPOS 0x06  /* if item in possession              */
#define OPC_IADD 0x07   /* add item to character's inventory  */
#define OPC_ALTER 0x08  /* alter map at coordinates           */
#define OPC_REDRAW 0x09 /* redraw screen                      */
#define OPC_ADDC 0x0a   /* add coins                          */
#define OPC_ADDE 0x0b   /* add experience                     */
#define OPC_SETREG 0x0c /* set register                       */
#define OPC_CLRENC 0x0d /* clear encounter                    */
#define OPC_ADDENC 0x0e /* add monsters to encounter row      */
#define OPC_DOENC 0x0f  /* do encounter                       */
#define OPC_ENTER 0x10  /* enter dungeon or wilderness        */
/* ---------------------------------------------------------- */

// clang-format off
#pragma code-name(push, "OVERLAY1");
// clang-format on

char signs[]= {
    0x20, // empty space
    0x5b, // diamond
    0x5c, // vertical door
    0x5d, // horizontal door
    0x5e, // filled space
    0x5f  // coat of arms
};

static dungeonDescriptor *desc= NULL;
unsigned int dungeonMapWidth;
byte lastFeelIndex;
byte registers[16];

#define R_PASS 15

byte currentX; // current coordinates inside map window
byte currentY;
int mposX; // current coords inside map
int mposY;
byte offsetX;
byte offsetY;

byte quitDungeon;

const byte mapWindowSize= 15;
const byte scrollMargin= 2;
const byte screenWidth= 40;
const byte screenX= 2;
const byte screenY= 2;

byte encounterWonOpcIdx;
byte encounterLostOpcIdx;

// prototypes

opcode *opcodeForIndex(byte idx);
dungeonItem *dungeonItemAtPos(byte x, byte y);
char *feelForIndex(byte idx);

void displayFeel(byte idx);
void redrawAll(void);
void plotPlayer(byte x, byte y);
void setupDungeonScreen(void);
void performOpcodeAtIndex(byte idx);
byte performOpcode(opcode *anOpcode, int dbgIdx);

// --------------------------------- opcodes ---------------------------------

// 0x01: NSTAT
byte performDisplayFeelOpcode(opcode *anOpcode) {
    byte feelIndex;

    feelIndex= anOpcode->param1;

    // make sure we display it only once
    if (feelIndex == lastFeelIndex) {
        return 0;
    }

    displayFeel(anOpcode->param1);
    lastFeelIndex= feelIndex;

    return 0;
}

// 0x02: DISP
byte performDisplayTextOpcode(opcode *anOpcode) {
    byte feelIndex;

    feelIndex= anOpcode->param1;
    if (anOpcode->param2 != 0) {
        clrscr();
    }
    // cprintf("%s",feelForIndex(anOpcode->param1));
    cputs(feelForIndex(anOpcode->param1));

    return 0;
}

// 0x03: WAITKEY
byte performWaitkeyOpcode(opcode *anOpcode) {
    performDisplayTextOpcode(anOpcode);
    while (kbhit()) {
        cgetc();
    }
    registers[anOpcode->param3]= cgetc();
    return 0;
}

// 0x04: YESNO
byte performYesNoOpcode(opcode *anOpcode) {
    byte inkey;
    cursor(true);
    do {
        inkey= cgetc();
    } while (inkey != 'y' && inkey != 'n');
    cursor(false);
    printf("%c\n", inkey);
    if (inkey == 'y') {
        registers[0]= true;
        performOpcodeAtIndex(anOpcode->param1);
    } else if (inkey == 'n') {
        registers[0]= false;
        performOpcodeAtIndex(anOpcode->param2);
    }
    return 0;
}

// 0x05: IFREG
byte performIfregOpcode(opcode *anOpcode) {

    byte regNr;
    byte value;
    regNr= anOpcode->param1;
    value= anOpcode->param2;

    if (anOpcode->id & 0x40) {
        if (registers[regNr] == value) {
            return anOpcode->param3;
        } else {
            return anOpcode->nextOpcodeIndex;
        }
    } else {
        if (registers[regNr] == value) {
            performOpcodeAtIndex(anOpcode->param3);
        } else {
            performOpcodeAtIndex(anOpcode->param4);
        }
    }
    return 0;
}

// 0x06: IFPOS
byte performIfposOpcode(opcode *anOpcode) {
    itemT anItemID;
    byte i;
    byte found;

    found= 0xff;
    anItemID= anOpcode->param1;

    for (i= 0; i < PARTYSIZE; ++i) {
        if (party[i]) {
            if (hasInventoryItem(party[i], anItemID)) {
                found= i;
                break;
            }
        }
    }

    registers[anOpcode->param4]= found;

    if (found != 0xff) {
        performOpcodeAtIndex(anOpcode->param2);
    } else {
        performOpcodeAtIndex(anOpcode->param3);
    }
    return 0;
}

// 0x07: IADD
byte performIAddOpcode(opcode *anOpcode) {
    byte charIdx;
    byte anItemID;
    byte found;

    anItemID= anOpcode->param1;
    charIdx= anOpcode->param2;
    found= false;

    if (charIdx == 0xff) {
        // choose first character with free inventory space
        for (charIdx= 0; charIdx < PARTYSIZE; ++charIdx) {
            if (party[charIdx]) {
                if (nextFreeInventorySlot(party[charIdx]) != 0xff) {
                    found= true;
                    break;
                }
            }
        }
        if (!found) {
            registers[0]= false; // failure flag
            performOpcodeAtIndex(anOpcode->param4);
            return 0;
        }
    }

    if (addInventoryItem(anItemID, party[charIdx])) {
        registers[0]= true;
        registers[1]= charIdx;
        if (anOpcode->id & 128) {
            cprintf("%s takes %s\r\n", party[charIdx]->name,
                    nameOfInventoryItemWithID(anItemID));
        }
        performOpcodeAtIndex(anOpcode->param3);
        return 0;
    }

    registers[0]= false;
    performOpcodeAtIndex(anOpcode->param4);
    return 0;
}

// 0x08: ALTER
byte performAlterOpcode(opcode *anOpcode) {
    dungeonItem *dItem;
    dItem= dungeonItemAtPos(anOpcode->param1, anOpcode->param2);
    dItem->opcodeID= anOpcode->param3;
    dItem->mapItem= anOpcode->param4;
    return 0;
}

// CAUTION: 0x0a has to be in the main segment
// because it is needed elsewhere as well..

// clang-format off
#pragma code-name(pop);
// clang-format on

// 0x0a: ADDC / ADDE / ADDC_V / ADDE_V
byte performAddCoinsOpcode(opcode *anOpcode) {
    byte opcodeID;
    int *coins;

    opcodeID= anOpcode->id & 31;

    coins= (int *)&(
        anOpcode->param1); // ...try to do something like that in Swift!

    if (opcodeID == 0x0a) {
        gPartyGold+= *coins;
        if (anOpcode->id & 128) {
            printf("The party gets %d coins\n", *coins);
        }

    } else {
        gPartyExperience+= *coins;
        if (anOpcode->id & 128) {
            printf("The party gets %d experience\n", *coins);
        }
    }
    return 0;
}

// clang-format off
#pragma code-name(push, "OVERLAY1");
// clang-format on

// 0x0c: SETREG
byte performSetregOpcode(opcode *anOpcode) {

    byte regNr;
    byte value;
    regNr= anOpcode->param1;
    value= anOpcode->param2;

    if (regNr > 16) {
        printf("??invalid register nr %d... quitting", regNr);
        exit(0);
    }

    registers[regNr]= value;
    return 0;
}

// 0x0d: CLEARENC
byte performClearencOpcode(void) {
    clearMonsters();
    return 0;
}

// 0x0e: ADDENC
byte performAddencOpcode(opcode *anOpcode) {

    if (anOpcode->param1) {
        addNewMonster(anOpcode->param1, anOpcode->param2, anOpcode->param3,
                      anOpcode->param4);
    }

    return 0;
}

// 0x0f DOENC
byte performDoencOpcode(opcode *anOpcode) {
    encounterWonOpcIdx=
        anOpcode->param1; // save result opcode indices for later on
    encounterLostOpcIdx= anOpcode->param2; // when re-entering dungeon module

    gEncounterResult= doPreEncounter(); // try pre-encounter first
    if (gEncounterResult == encFight) {
        prepareForGameMode(gm_encounter);
        quitDungeon= true;
        /*
             a real fight? -->
             quit dungeon and let dispatcher handle loading the
             rest of the encounter module
        */
    } else {
        redrawAll();
    }
    return 0;
}

// 0x10: ENTER_D / ENTER_W
byte performEnterOpcode(opcode *anOpcode) {
    byte opcodeID;
    byte x, y;
    gameModeT newGameMode;

    opcodeID= anOpcode->id & 31;
    gCurrentDungeonIndex= anOpcode->param1;
    gOutdoorXPos= anOpcode->param2;
    gOutdoorYPos= anOpcode->param3;

    newGameMode = (anOpcode->id & 32) ? gm_dungeon : gm_outdoor;

    if (newGameMode == gm_outdoor) {
        gCurrentDungeonIndex |= 128; // outdoor maps have bit 7 set in their index
    }

    #ifdef DEBUG
        printf("ENTER with game mode %d",newGameMode);
        cgetc();
    #endif

    prepareForGameMode(newGameMode);
    quitDungeon = true;
    return 0;
}

// ---------------------------------
// general opcode handling functions
// ---------------------------------

void performOpcodeAtIndex(byte idx) {

    byte next;
    next= idx;

    do {
        next= performOpcode(opcodeForIndex(next), next);
    } while (next);
}

byte performOpcode(opcode *anOpcode, int dbgIdx) {

    byte xs, ys; // x,y save
    byte opcodeID;
    byte nextOpcodeIndex;

    byte rOpcIdx; // returned opcode index from singular opcodes, used for
                  // branching

#ifdef DEBUG
    xs= wherex();
    ys= wherey();
    gotoxy(0, 24);
    printf("%02x:%02x%02x%02x%02x%02x%02x%02x>%02x", dbgIdx, anOpcode->id,
           anOpcode->param1, anOpcode->param2, anOpcode->param3,
           anOpcode->param4, anOpcode->param5, anOpcode->param6,
           anOpcode->nextOpcodeIndex); // DEBUG
    gotoxy(0, 23);
    gotoxy(xs, ys);
#endif

    opcodeID= (anOpcode->id) & 31; // we have 5-bit opcodes from 0x00 - 0x1f
    nextOpcodeIndex= anOpcode->nextOpcodeIndex;

    switch (opcodeID) {

    case OPC_NOP:
        rOpcIdx= 0;
        break;

    case OPC_NSTAT:
        rOpcIdx= performDisplayFeelOpcode(anOpcode);
        break;

    case OPC_DISP:
        rOpcIdx= performDisplayTextOpcode(anOpcode);
        break;

    case OPC_WKEY:
        rOpcIdx= performWaitkeyOpcode(anOpcode);
        break;

    case OPC_YESNO:
        rOpcIdx= performYesNoOpcode(anOpcode);
        break;

    case OPC_IFREG:
        rOpcIdx= performIfregOpcode(anOpcode);
        break;

    case OPC_IFPOS:
        rOpcIdx= performIfposOpcode(anOpcode);
        break;

    case OPC_IADD:
        rOpcIdx= performIAddOpcode(anOpcode);
        break;

    case OPC_ALTER:
        rOpcIdx= performAlterOpcode(anOpcode);
        break;

    case OPC_REDRAW:
        redrawAll();
        rOpcIdx= 0;
        break;

    case OPC_ADDC:
        rOpcIdx= performAddCoinsOpcode(anOpcode);
        break;

    case OPC_ADDE:
        rOpcIdx= performAddCoinsOpcode(anOpcode);
        break;

    case OPC_SETREG:
        rOpcIdx= performSetregOpcode(anOpcode);
        break;

    case OPC_CLRENC:
        rOpcIdx= performClearencOpcode();
        break;

    case OPC_ADDENC:
        rOpcIdx= performAddencOpcode(anOpcode);
        break;

    case OPC_DOENC:
        rOpcIdx= performDoencOpcode(anOpcode);
        break;

    case OPC_ENTER:
        rOpcIdx= performEnterOpcode(anOpcode);
        return (NULL); // enter *always* quits the current dungeon

    default:
        rOpcIdx= 0;
        break;
    }

    if (anOpcode->id & 0x40) {    // do we have a branch opcode?
        nextOpcodeIndex= rOpcIdx; // use returned index as next index
    }

    return nextOpcodeIndex;
}

dungeonItem *dungeonItemAtPos(byte x, byte y) {
    return desc->dungeon + x + (y * dungeonMapWidth);
}

void redrawMap() { blitmap(offsetX, offsetY, screenX, screenY); }

void redrawAll() {
    setupDungeonScreen();
    redrawMap();
    showCurrentParty(true);
    plotPlayer(currentX, currentY);
    if (lastFeelIndex != 255) {
        displayFeel(lastFeelIndex);
    }
}

void plotDungeonItem(dungeonItem *item, byte x, byte y) {

    register byte *screenPtr; // working pointer to screen

    screenPtr= SCREEN + (screenWidth * screenY) + screenX;
    screenPtr+= x + (y * screenWidth);
    *screenPtr= signs[item->mapItem & 15];
}

void plotPlayer(byte x, byte y) {
    register byte *screenPtr; // working pointer to screen

    screenPtr= SCREEN + (screenWidth * screenY) + screenX;
    screenPtr+= x + (y * screenWidth);

    *screenPtr= signs[5];
}

void displayFeel(byte feelID) {
    cg_clearLower(5);
    gotoxy(0, 19);
    puts(feelForIndex(feelID));
}

// moves origin and redraws map if player is in scroll area
void ensureSaneOffset() {

    if (currentX > (mapWindowSize - scrollMargin - 1)) {
        if (offsetX + mapWindowSize < dungeonMapWidth) {
            ++offsetX;
            --currentX;
            redrawMap();
        }
    }

    if (currentX < scrollMargin + 1) {
        if (offsetX > 0) {
            --offsetX;
            ++currentX;
            redrawMap();
        }
    }

    if (currentY > mapWindowSize - scrollMargin - 1) {
        if (offsetY + mapWindowSize < desc->dungeonMapHeight) {
            ++offsetY;
            --currentY;
            redrawMap();
        }
    }

    if (currentY < scrollMargin + 1) {
        if (offsetY > 0) {
            --offsetY;
            ++currentY;
            redrawMap();
        }
    }
}

/*
breseham's line drawing algorithm for
field-of-view calculation
*/

void look_bh(int x0, int y0, int x1, int y1) {

    int dx, dy;
    int x, y;
    int n;
    int x_inc, y_inc;
    int error;

    dungeonItem *anItem;
    byte plotSign;

    dx= abs(x1 - x0);
    dy= abs(y1 - y0);
    x= x0;
    y= y0;
    n= 1 + dx + dy;
    x_inc= (x1 > x0) ? 1 : -1;
    y_inc= (y1 > y0) ? 1 : -1;
    error= dx - dy;

    dx*= 2;
    dy*= 2;

    for (; n > 0; --n) {

        anItem= dungeonItemAtPos(x, y);
        plotSign= anItem->mapItem & 15;

        seenMap[x + (dungeonMapWidth * y)]= anItem->mapItem;

        if (plotSign >= 2) {
            if (x != x0 || y != y0)
                return;
        }

        if (error > 0) {
            x+= x_inc;
            error-= dy;
        } else {
            y+= y_inc;
            error+= dx;
        }
    }
}

#define L_DISTANCE 4

// perform bresenham for 4 edges of f-o-v
void look(int x, int y) {
    signed char xi1, xi2, yi1, yi2;

    yi1= y - L_DISTANCE;
    if (yi1 < 0)
        yi1= 0;

    yi2= y + L_DISTANCE;
    if (yi2 > desc->dungeonMapHeight)
        yi2= desc->dungeonMapHeight;

    for (xi1= x - L_DISTANCE; xi1 <= x + L_DISTANCE; ++xi1) {
        look_bh(x, y, xi1, yi1);
        look_bh(x, y, xi1, yi2);
    }

    xi1= x - L_DISTANCE;
    if (xi1 < 0)
        xi1= 0;

    xi2= x + L_DISTANCE;
    if (xi2 > dungeonMapWidth)
        xi2= dungeonMapWidth;

    for (yi1= y - L_DISTANCE; yi1 <= y + L_DISTANCE; ++yi1) {
        look_bh(x, y, xi1, yi1);
        look_bh(x, y, xi2, yi1);
    }

    redrawMap();
}

void dungeonLoop() {

    byte xs, ys;     // save x,y for debugging
    byte oldX, oldY; // save x,y for impassable

    byte cmd;
    byte performedImpassableOpcode;

    signed char xdiff, ydiff;

    dungeonItem *dItem;
    dungeonItem *currentItem;

    redrawAll();
    performedImpassableOpcode= false;

    /**************************************************************
     *
     * dloop starts here
     *
     **************************************************************/

    while (!quitDungeon) {

        ensureSaneOffset();

        // draw player surrounding

        for (xdiff= -1; xdiff <= 1; xdiff++) {
            for (ydiff= -1; ydiff <= 1; ydiff++) {
                mposX= currentX + offsetX + xdiff;
                mposY= currentY + offsetY + ydiff;
                if (mposX >= 0 && mposY >= 0 && mposX < dungeonMapWidth &&
                    mposY < desc->dungeonMapHeight) {
                    dItem= dungeonItemAtPos(mposX, mposY);
                    if (xdiff == 0 && ydiff == 0) {
                        currentItem= dItem;
                        plotPlayer(currentX, currentY);
                    } else {
                        seenMap[mposX + (dungeonMapWidth * mposY)]=
                            dItem->mapItem;
                        plotDungeonItem(dItem, currentX + xdiff,
                                        currentY + ydiff);
                    }
                }
            }
        }

        if (gEncounterResult !=
            encUndef) { // coming from an encounter? --> handle result first

            if ((gEncounterResult == encWon ||
                 gEncounterResult == encSurrender ||
                 gEncounterResult == encGreet) &&
                encounterWonOpcIdx) {

                performOpcodeAtIndex(encounterWonOpcIdx);
            }

            else if ((gEncounterResult == encFled ||
                      gEncounterResult == encMercy) &&
                     encounterLostOpcIdx) {

                performOpcodeAtIndex(encounterLostOpcIdx);
            } else {

                gEncounterResult=
                    encUndef; // reset global encounter result for next time...
            }
        }

        // *** perform opcode for this position! **
        if (!performedImpassableOpcode) {
            performOpcodeAtIndex(currentItem->opcodeID);
        }

        performedImpassableOpcode= false;

        oldX= currentX;
        oldY= currentY;

        cmd= 0;

        if (!quitDungeon) {

            cmd= cgetc();

            if (cmd >= '1' && cmd <= '6' && cmd != 'l') {
                inspectCharacter(cmd - '1');
                redrawAll();
            }
        }

        switch (cmd) {

        case 'l':
            look(currentX + offsetX, currentY + offsetY);
            break;

        case 29: // cursor right
            currentX++;
            break;

        case 157: // cursor left
            currentX--;
            break;

        case 145: // cursor up
            currentY--;
            break;

        case 17: // cursor down
            currentY++;
            break;

        case 'q':
            quitDungeon= true;

        default:
            break;
        }

        mposX= currentX + offsetX;
        mposY= currentY + offsetY;

        // can we move here at all?
        if (mposX < 0 || mposY < 0 || mposX >= dungeonMapWidth ||
            mposY >= desc->dungeonMapHeight) {
            // nope, reset position
            currentX= oldX;
            currentY= oldY;
        } else {
            // what is here?
            dItem= dungeonItemAtPos(mposX, mposY);

#ifdef DEBUG
            xs= wherex();
            ys= wherey();
            gotoxy(27, 24);
            printf("%2d,%2d: %02x %02x", mposX, mposY, dItem->opcodeID,
                   dItem->mapItem);
            gotoxy(xs, ys);
#endif

            if (dItem->mapItem & 32) { // check impassable flag
                // can't go there: reset pass register...
                registers[R_PASS]= 255;
                // ...perform opcode...
                performOpcodeAtIndex(dItem->opcodeID);
                performedImpassableOpcode= true;
                // ...and check if 'pass' register has become valid...
                if (registers[R_PASS] == 255) {
                    // ...you can not pass
                    currentX= oldX;
                    currentY= oldY;
                }
            }
        }

    } // of while (!quitDungeon);
}

opcode *opcodeForIndex(byte idx) { return desc->opcodesAdr + idx; }

char *feelForIndex(byte idx) { return desc->feelTbl[idx]; }

void setupDungeonScreen(void) {
    byte x;
    textcolor(COLOR_BLACK);
    clrscr();
    bordercolor(BCOLOR_WHITE | CATTR_LUMA3);
    bgcolor(BCOLOR_WHITE | CATTR_LUMA6);
    textcolor(BCOLOR_BLUEGREEN | CATTR_LUMA1);
    revers(1);

    for (x= 0; x < mapWindowSize + 1; ++x) {
        cputcxy(screenX + x, screenY - 1, ' ');
        cputcxy(screenX - 1 + x, screenY + mapWindowSize, ' ');
        cputcxy(screenX - 1, screenY - 1 + x, ' ');
        cputcxy(screenX + mapWindowSize, screenY + x, ' ');
    }
    revers(0);

    textcolor(COLOR_BLACK);
}

void unloadDungeon(void) {

    if (desc != NULL) {
        free(desc->dungeon);
        free(desc->feelTbl);
        free(desc->mapdata);
        desc= NULL;
    }

    gLoadedDungeonIndex= 255;
}

void loadNewDungeon(void) {

    char *mfile;
    char *dungeonFile= "mapa";
    char *outdoorFile= "outa";

    mfile= gCurrentGameMode == gm_dungeon ? dungeonFile : outdoorFile;

    unloadDungeon();

    mfile[3]= 'a' + (gCurrentDungeonIndex & 127);
    
    desc= loadMap(mfile);

    if (!desc) {
        puts("could not load dungeon");
        exit(0);
    }

    encounterLostOpcIdx= 0;
    encounterWonOpcIdx= 0;

    gEncounterResult= encUndef;

    dungeonMapWidth= desc->dungeonMapWidth;
    gLoadedDungeonIndex= gCurrentDungeonIndex;

    lastFeelIndex= 0;

    currentX= desc->startX;
    currentY= desc->startY;
    offsetX= 0;
    offsetY= 0;
    mposX= 0;
    mposY= 0;

    offsetX= currentX - (mapWindowSize / 2);
    currentX= mapWindowSize / 2;

    if (currentY > mapWindowSize) {
        offsetY= currentY - (mapWindowSize / 2) - 1;
        currentY= mapWindowSize / 2;
    }
}

void enterDungeonMode(void) {
#ifdef DEBUG
    if (gCurrentGameMode == gm_dungeon) {
        puts("entering dungeon mode");
    } else if (gCurrentGameMode == gm_outdoor) {
        puts("entering outdoor mode");
    } else {
        puts("??unknown game mode in dungeon!");
        exit(0);
    }
#endif
    if (gLoadedDungeonIndex != gCurrentDungeonIndex) {
        loadNewDungeon();
    }
    quitDungeon= false;
    dungeonLoop();
}

void blitmap(byte mapX, byte mapY, byte posX, byte posY) {

    register byte *screenPtr; // working pointer to screen
    register byte *bufPtr;    // working pointer to line buffer
    byte *seenMapPtr;
    unsigned int offset;
    byte mapItem;
    byte xs, ys;

    byte screenStride;
    byte mapStride;
    dungeonItem *dungeon;

    dungeon= desc->dungeon;
    screenStride= screenWidth - mapWindowSize;
    mapStride= dungeonMapWidth;

    screenPtr= SCREEN + (screenWidth * posY) + posX;
    seenMapPtr= seenMap + (mapY * dungeonMapWidth) + mapX - 1;

    offset= 0;

    for (ys= 0; ys < mapWindowSize; ++ys) {
        bufPtr= seenMapPtr + offset;
        for (xs= 0; xs < mapWindowSize; ++xs, ++screenPtr) {
            mapItem= *(++bufPtr);
            if (mapItem != 255) {
                *screenPtr= signs[mapItem & 15];
            } else {
                *screenPtr= 160;
            }
        }
        screenPtr+= screenStride;
        offset+= desc->dungeonMapWidth;
    }
}

// clang-format off
#pragma code-name(pop);
// clang-format on
