#include "dungeon.h"
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

#define ClearBit(A, k) (*(A + (k / 8))&= ~(1 << (k % 8)))
#define TestBit(A, k) (*(A + (k / 8)) & (1 << (k % 8)))

char signs[]= {
    0x60, // empty space
    123,  // diamond
    0x5d, // vertical door<i
    0x40, // horizontal door
    102   // filled space
};

/*
byte *seenSpaces;
dungeonItem *dungeon;
byte *dungeonMapWidth;
byte *dungeonMapHeight;
byte *startX;
byte *startY;
*/

dungeonDescriptor *desc;

byte lastFeelIndex;

byte currentX; // current coordinates inside map window
byte currentY;
byte offsetX;
byte offsetY;

// clang-format off
#pragma codesize(push, 300);
// clang-format on

const byte mapWindowSize= 15;
const byte scrollMargin= 2;
const byte screenWidth= 40;
const byte screenX= 2;
const byte screenY= 2;

opcode *opcodeForIndex(byte idx);

char *feelForIndex(byte idx);

void SetBit(byte *A, int k) {
    byte i= k / 8;
    byte pos= k % 8;

    byte flag= 1;

    flag= flag << pos;

    *(A + i)= *(A + i) | flag;
}

dungeonItem *dungeonItemAtPos(byte x, byte y) {
    return desc->dungeon + x + (y * desc->dungeonMapWidth);
}

void redrawMap() { blitmap(offsetX, offsetY, screenX, screenY); }

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
    *screenPtr= 0;
}

void performDisplayFeelOpcode(opcode *anOpcode) {
    byte i;
    byte feelIndex;

    feelIndex= anOpcode->param1;

    // make sure we display it only once
    if (feelIndex == lastFeelIndex) {
        return;
    }

    for (i= 20; i < 24; ++i) {
        cclearxy(0, i, 40);
    }
    gotoxy(0, 20);
    puts(feelForIndex(anOpcode->param1));
    lastFeelIndex= feelIndex;
}

void performOpcode(opcode *anOpcode) {

#ifdef DEBUG
    gotoxy(0, 24);
    printf("opc%02x-%02x%02x%02x%02x%02x%02x.%02x", anOpcode->id,
           anOpcode->param1, anOpcode->param2, anOpcode->param3,
           anOpcode->param4, anOpcode->param5, anOpcode->param6,
           anOpcode->nextOpcode); // DEBUG
#endif

    if (anOpcode->id == 0x01) {
        performDisplayFeelOpcode(anOpcode);
    }
}

// moves origin and redraws map if player is in scroll area
void ensureSaneOffset() {

    if (currentX > (mapWindowSize - scrollMargin - 1)) {
        if (offsetX + mapWindowSize < desc->dungeonMapWidth) {
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

void dungeonLoop() {

    int mposX; // current coords inside map
    int mposY;

    opcode *op; // current opcode

    byte oldX, oldY;

    byte cmd;
    byte quit;

    signed char xdiff, ydiff;

    dungeonItem *dItem;
    dungeonItem *currentItem;

    quit= 0;
    lastFeelIndex= 0;

    currentX= desc->startX - 1;
    currentY= desc->startY - 1;
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

    redrawMap();

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
                if (mposX >= 0 && mposY >= 0 && mposX < desc->dungeonMapWidth &&
                    mposY < desc->dungeonMapHeight) {
                    dItem= dungeonItemAtPos(mposX, mposY);
                    if (xdiff == 0 && ydiff == 0) {
                        currentItem= dItem;
                        plotPlayer(currentX, currentY);
                    } else {
                        SetBit(desc->seenSpaces,
                               mposX + (desc->dungeonMapWidth * mposY));
                        plotDungeonItem(dItem, currentX + xdiff,
                                        currentY + ydiff);
                    }
                }
            }
        }

        op= opcodeForIndex(currentItem->opcodeID);
        performOpcode(op);

        oldX= currentX;
        oldY= currentY;

        cmd= cgetc();

        switch (cmd) {

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
        if (mposX < 0 || mposY < 0 || mposX >= desc->dungeonMapWidth ||
            mposY >= desc->dungeonMapHeight) {
            // nope, reset position
            currentX= oldX;
            currentY= oldY;
        } else {
            // what is here?
            dItem= dungeonItemAtPos(mposX, mposY);
#ifdef DEBUG
            gotoxy(27, 24);
            printf("%2d,%2d: %02x %02x", mposX, mposY, dItem->opcodeID,
                   dItem->mapItem); // DEBUG
#endif
            if (dItem->mapItem & 32) { // check impassable flag
                // can't go there
                currentX= oldX;
                currentY= oldY;
                // but perform opcode if anything's there
                op= opcodeForIndex(dItem->opcodeID);
                performOpcode(op);
            }
        }
    } while (!quit);
}

opcode *opcodeForIndex(byte idx) { return desc->opcodesAdr + idx; }

char *feelForIndex(byte idx) { return desc->feelTbl[idx]; }

void setupDungeonScreen(void) {
    clrscr();
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
    cgetc();
    textcolor(COLOR_BLACK);
    bordercolor(BCOLOR_WHITE | CATTR_LUMA3);
    bgcolor(BCOLOR_WHITE | CATTR_LUMA6);
    setupDungeonScreen();
    dungeonLoop();
}

void blitmap(byte mapX, byte mapY, byte posX, byte posY) {

    register byte *screenPtr;     // working pointer to screen
    register dungeonItem *bufPtr; // working pointer to line buffer
    register dungeonItem *dunStartPtr;
    unsigned int seenStartIdx;
    unsigned int seenIdx;
    unsigned int offset;
    byte mapItem;
    byte xs, ys;

    byte screenStride;
    byte mapStride;
    dungeonItem *dungeon;

    dungeon= desc->dungeon;
    screenStride= screenWidth - mapWindowSize;
    mapStride= desc->dungeonMapWidth;

    screenPtr= SCREEN + (screenWidth * posY) + posX;
    dunStartPtr= dungeon + (mapY * desc->dungeonMapWidth) + mapX - 1;
    seenStartIdx= (mapY * desc->dungeonMapWidth) + mapX;

    offset= 0;

    for (ys= 0; ys < mapWindowSize; ++ys) {
        bufPtr= dunStartPtr + offset;
        seenIdx= seenStartIdx + offset;
        for (xs= 0; xs < mapWindowSize; ++xs, ++screenPtr, ++seenIdx) {
            mapItem= (++bufPtr)->mapItem;
            if ((mapItem & 128) ||
                TestBit(desc->seenSpaces, seenIdx)) { // visible/automapped?
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
// clang-format on
