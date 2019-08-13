#include "dungeon.h"
#include "character.h"
#include "dungeonLoader.h"
#include "types.h"
#include <conio.h>
#include <plus4.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 128

#define LOWBYTE(v) ((unsigned char)(v))
#define HIGHBYTE(v) ((unsigned char)(((unsigned int)(v)) >> 8))
#define SCREEN ((unsigned char *)0xc00)

// #define ClearBit(A, k) (*(A + (k / 8))&= ~(1 << (k % 8)))
// #define TestBit(A, k) (*(A + (k / 8)) & (1 << (k % 8)))

/* ------------------------ opcodes ------------------------- */
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
/* ---------------------------------------------------------- */

// clang-format off
#pragma code-name(push, "OVERLAY1");
// clang-format on

char signs[]= {
    0x60, // empty space
    0x61, // diamond
    0x62, // vertical door
    0x63, // horizontal door
    0x64, // filled space
    0x65  // coat of arms
};

dungeonDescriptor *desc;
unsigned int dungeonMapWidth;
byte lastFeelIndex;
byte registers[16];

#define R_PASS 15

byte currentX; // current coordinates inside map window
byte currentY;
byte offsetX;
byte offsetY;

const byte mapWindowSize= 15;
const byte scrollMargin= 2;
const byte screenWidth= 40;
const byte screenX= 2;
const byte screenY= 2;

// prototypes

opcode *opcodeForIndex(byte idx);
dungeonItem *dungeonItemAtPos(byte x, byte y);
char *feelForIndex(byte idx);

void displayFeel(byte idx);
void redrawAll(void);
void plotPlayer(byte x, byte y);
void setupDungeonScreen(void);
void performOpcodeAtIndex(byte idx);
void performOpcode(opcode *anOpcode);

// --------------------------------- opcodes ---------------------------------

// 0x01: NSTAT
void performDisplayFeelOpcode(opcode *anOpcode) {
    byte feelIndex;

    feelIndex= anOpcode->param1;

    // make sure we display it only once
    if (feelIndex == lastFeelIndex) {
        return;
    }

    displayFeel(anOpcode->param1);
    lastFeelIndex= feelIndex;
}

// 0x02: DISP
void performDisplayTextOpcode(opcode *anOpcode) {
    byte feelIndex;

    feelIndex= anOpcode->param1;
    if (anOpcode->param2 != 0) {
        clrscr();
    }
    fputs(feelForIndex(anOpcode->param1), stdout);
}

// 0x03: WAITKEY
void performWaitkeyOpcode(opcode *anOpcode) {
    performDisplayTextOpcode(anOpcode);
    registers[anOpcode->param3]= cgetc();
}

// 0x04: YESNO
void performYesNoOpcode(opcode *anOpcode) {
    byte inkey;
    cursor(true);
    do {
        inkey= cgetc();
    } while (inkey != 'y' && inkey != 'n');
    cursor(false);
    cprintf("%c\r\n", inkey);
    if (inkey == 'y') {
        registers[0]= true;
        performOpcodeAtIndex(anOpcode->param1);
    } else if (inkey == 'n') {
        registers[0]= false;
        performOpcodeAtIndex(anOpcode->param2);
    }
}

// 0x05: IFREG
void performIfregOpcode(opcode *anOpcode) {
    byte regNr;
    byte value;
    regNr= anOpcode->param1;
    value= anOpcode->param2;
    if (registers[regNr] == value) {
        performOpcodeAtIndex(anOpcode->param3);
    } else {
        performOpcodeAtIndex(anOpcode->param4);
    }
}

// 0x06: IFPOS
void performIfposOpcode(opcode *anOpcode) {
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
}

// 0x07: IADD
void performIAddOpcode(opcode *anOpcode) {
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
            return;
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
        return;
    }

    registers[0]= false;
    performOpcodeAtIndex(anOpcode->param4);
    return;
}

// 0x08: ALTER
void performAlterOpcode(opcode *anOpcode) {
    dungeonItem *dItem;
    dItem= dungeonItemAtPos(anOpcode->param1, anOpcode->param2);
    dItem->opcodeID= anOpcode->param3;
    dItem->mapItem= anOpcode->param4;
}

// 0x0a: ADDC
void performAddCoinsOpcode(opcode *anOpcode) {
    byte charIdx;
    int *coins;
    int numMembers;
    int coinsPerMember;

    numMembers= partyMemberCount();
    coins= (int *)&(
        anOpcode->param1); // ...try to do something like that in Swift!
    coinsPerMember= *coins / numMembers;

    for (charIdx= 0; charIdx < PARTYSIZE; ++charIdx) {
        if (party[charIdx]) {
            party[charIdx]->gold+= coinsPerMember;
        }
    }

    if (anOpcode->id & 128) {
        cprintf("The party gets %d coins\r\n", coinsPerMember * numMembers);
    }
}

// ---------------------------------
// general opcode handling functions
// ---------------------------------

void performOpcodeAtIndex(byte idx) {

    if (idx == 0)
        return;

    /*     if ((opcodeForIndex(idx)->id)>1) {
          gotoxy(0,0);
          cprintf("-- i%d:%d --",idx,opcodeForIndex(idx)->id); cgetc();
      }
      */

    performOpcode(opcodeForIndex(idx));
}

void performOpcode(opcode *anOpcode) {

    byte xs, ys; // x,y save
    byte opcodeID;

#ifdef DEBUG
    xs= wherex();
    ys= wherey();
    gotoxy(0, 24);
    printf("opc%02x.%02x%02x%02x%02x%02x%02x>%02x", anOpcode->id,
           anOpcode->param1, anOpcode->param2, anOpcode->param3,
           anOpcode->param4, anOpcode->param5, anOpcode->param6,
           anOpcode->nextOpcode); // DEBUG
    gotoxy(xs, ys);
    // cgetc();
#endif

    opcodeID= (anOpcode->id) & 127;

    switch (opcodeID) {

    case OPC_NSTAT:
        performDisplayFeelOpcode(anOpcode);
        break;

    case OPC_DISP:
        performDisplayTextOpcode(anOpcode);
        break;

    case OPC_WKEY:
        performWaitkeyOpcode(anOpcode);
        break;

    case OPC_YESNO:
        performYesNoOpcode(anOpcode);
        break;

    case OPC_IFREG:
        performIfregOpcode(anOpcode);
        break;

    case OPC_IFPOS:
        performIfposOpcode(anOpcode);
        break;

    case OPC_IADD:
        performIAddOpcode(anOpcode);
        break;

    case OPC_ALTER:
        performAlterOpcode(anOpcode);
        break;

    case OPC_REDRAW:
        redrawAll();
        break;

    case OPC_ADDC:
        performAddCoinsOpcode(anOpcode);
        break;

    default:
        break;
    }

    if (anOpcode->nextOpcode != OPC_NOP) {
        performOpcodeAtIndex(anOpcode->nextOpcode);
    }
}

// ----------------------------------------------------------------------------

void SetBit(byte *A, int k) {
    byte i= k / 8;
    byte pos= k % 8;

    byte flag= 1;

    flag= flag << pos;

    *(A + i)= *(A + i) | flag;
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
    *screenPtr= 0x65;
}

void displayFeel(byte feelID) {

    byte i;

    for (i= 19; i < 24; ++i) {
        cclearxy(0, i, 40);
    }
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

// clang-format off
#pragma codesize(push, 300);
// clang-format on

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

        seenMap[x+(dungeonMapWidth*y)] = anItem->mapItem;

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
    int xi1, xi2, yi1, yi2;

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

    int mposX; // current coords inside map
    int mposY;

    byte xs, ys; // save x,y for debugging

    byte oldX, oldY; // save x,y for impassable

    byte cmd;
    byte quit;
    byte performedImpassableOpcode;

    signed char xdiff, ydiff;

    dungeonItem *dItem;
    dungeonItem *currentItem;

    quit= 0;
    lastFeelIndex= 0;

    currentX= desc->startX;
    currentY= desc->startY;
    offsetX= 0;
    offsetY= 0;
    mposX= 0;
    mposY= 0;

    // if (currentX > mapWindowSize) {
    offsetX= currentX - (mapWindowSize / 2);
    currentX= mapWindowSize / 2;
    //}

    if (currentY > mapWindowSize) {
        offsetY= currentY - (mapWindowSize / 2) - 1;
        currentY= mapWindowSize / 2;
    }

    redrawAll();
    performedImpassableOpcode= false;

    /**************************************************************
     *
     * dloop starts here
     *
     **************************************************************/

    do {

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
                        seenMap[mposX+(dungeonMapWidth*mposY)] = dItem->mapItem;
                        plotDungeonItem(dItem, currentX + xdiff,
                                        currentY + ydiff);
                    }
                }
            }
        }

        if (!performedImpassableOpcode) {
            performOpcodeAtIndex(currentItem->opcodeID);
        }

        performedImpassableOpcode= false;

        oldX= currentX;
        oldY= currentY;

        cmd= cgetc();

        if (cmd >= '1' && cmd <= '6' && cmd != 'l') {
            inspectCharacter(cmd - '1');
            redrawAll();
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
            quit= 1;

        default:
            break;
        }

        mposX= currentX + offsetX;
        mposY= currentY + offsetY;

        // can we move here?
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
                // ...and check 'pass' register
                if (registers[R_PASS] == 255) {
                    currentX= oldX;
                    currentY= oldY;
                }
            }
        }
    } while (!quit);
}

opcode *opcodeForIndex(byte idx) { return desc->opcodesAdr + idx; }

char *feelForIndex(byte idx) { return desc->feelTbl[idx]; }

void setupDungeonScreen(void) {
    textcolor(COLOR_BLACK);
    clrscr();
    bordercolor(BCOLOR_WHITE | CATTR_LUMA3);
    bgcolor(BCOLOR_WHITE | CATTR_LUMA6);
    textcolor(BCOLOR_BLUEGREEN | CATTR_LUMA1);
    cputcxy(screenX - 1, screenY - 1, 176);             // left upper corner
    cputcxy(screenX + mapWindowSize, 1, 174);           // right upper corner
    cputcxy(screenX - 1, screenY + mapWindowSize, 173); // left lower corner
    cputcxy(screenX + mapWindowSize, screenY + mapWindowSize,
            189); // right lower corner
    chlinexy(screenX, screenY - 1, mapWindowSize);
    chlinexy(screenX, screenY + mapWindowSize, mapWindowSize);
    cvlinexy(screenX - 1, screenY, mapWindowSize);
    cvlinexy(screenX + mapWindowSize, screenY, mapWindowSize);
    textcolor(COLOR_BLACK);
}

void testMap(void) {
    clrscr();
    cprintf("**mapdebug**\r\n");
    desc= loadMap("map0");
    dungeonMapWidth= desc->dungeonMapWidth;
    cgetc();
    dungeonLoop();
}

void blitmap(byte mapX, byte mapY, byte posX, byte posY) {

    register byte *screenPtr;     // working pointer to screen
    register byte *bufPtr;        // working pointer to line buffer
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
#pragma codesize(pop);
#pragma code-name(pop);
// clang-format on
