#include "dungeon.h"
#include "types.h"
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 128

#define LOWBYTE(v) ((unsigned char)(v))
#define HIGHBYTE(v) ((unsigned char)(((unsigned int)(v)) >> 8))
#define SCREEN ((unsigned char *)0x800)

#define SetBit(A, k) (A[(k / 8)]|= (1 << (k % 8)))
#define ClearBit(A, k) (A[(k / 8)]&= ~(1 << (k % 8)))
#define TestBit(A, k) (A[(k / 8)] & (1 << (k % 8)))

char signs[]= {
    0x60, // empty space
    123,  // diamond
    0x5d, // vertical door
    0x40, // horizontal door
    102   // filled space
};

byte *seenSpaces;

byte *mapdata;
byte *dungeon;
byte *dungeonMapWidth;
byte *dungeonMapHeight;
byte *startX;
byte *startY;

byte linebuf[BUFSIZE];
byte numFeels;
byte numOpcs;

unsigned int dungeonSize;

byte lastFeelIndex;

byte currentX; // current coordinates inside map window
byte currentY;

byte offsetX;
byte offsetY;

unsigned char **feelTbl; // pointer to feel addresses
opcode *opcodesAdr;      // external address of 1st opcode

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

unsigned int dungeonItemAtPos(byte x, byte y) {
    return dungeon[(x * 2) + (y * *dungeonMapWidth * 2)];
}

void redrawMap() { blitmap(offsetX, offsetY, screenX, screenY); }

void plotDungeonItem(unsigned int dItem, byte x, byte y) {

    register byte *screenPtr; // working pointer to screen

    screenPtr= SCREEN + (screenWidth * screenY) + screenX;

    screenPtr+= x;
    screenPtr+= (y * screenWidth);

    *screenPtr= signs[dItem & 15];
}

void plotPlayer(byte x, byte y) {
    register byte *screenPtr; // working pointer to screen

    screenPtr= SCREEN + (screenWidth * screenY) + screenX;

    screenPtr+= x;
    screenPtr+= (y * screenWidth);

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
    if (anOpcode->id == 0x01) {
        performDisplayFeelOpcode(anOpcode);
    }
}

// moves origin and redraws map if player is in scroll area
void ensureSaneOffset() {

    if (currentX > (mapWindowSize - scrollMargin - 1)) {
        if (offsetX + mapWindowSize < *dungeonMapWidth) {
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
        if (offsetY + mapWindowSize < *dungeonMapHeight) {
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

    unsigned int dItem;
    unsigned int currentItem;

    quit= 0;
    currentX= *startX - 1;
    currentY= *startY - 1;
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
                if (mposX >= 0 && mposY >= 0 && mposX < *dungeonMapWidth &&
                    mposY < *dungeonMapHeight) {
                    dItem= dungeonItemAtPos(mposX, mposY);
                    if (xdiff == 0 && ydiff == 0) {
                        currentItem= dItem;
                        plotPlayer(currentX + xdiff, currentY + ydiff);
                    } else {
                        plotDungeonItem(dItem, currentX + xdiff,
                                        currentY + ydiff);
                    }
                }
            }
        }

        op= opcodeForIndex(HIGHBYTE(currentItem));
        performOpcode(op);

        oldX= currentX;
        oldY= currentY;

        cmd= cgetc();

        switch (cmd) {
        case 'l':
            currentX++;
            break;

        case 'j':
            currentX--;
            break;

        case 'i':
            currentY--;
            break;

        case 'k':
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
        if (mposX < 0 || mposY < 0 || mposX > *dungeonMapWidth ||
            mposY > *dungeonMapHeight) {
            // nope, reset position
            currentX= oldX;
            currentY= oldY;
        } else {
            // what is here?
            dItem= dungeonItemAtPos(mposX, mposY);
#ifdef DEBUG
            gotoxy(35, 24);
            printf("%04x", dItem); // DEBUG
#endif
            if (dItem & 32) { // check impassable flag
                // can't go there
                currentX= oldX;
                currentY= oldY;
            }
        }
    } while (!quit);
}

opcode *opcodeForIndex(byte idx) { return opcodesAdr + (8 * idx); }

char *feelForIndex(byte idx) { return feelTbl[idx]; }

byte *buildFeelsTable(byte *startAddr) {
    byte *currentPtr; // currentExternalAdr;
    unsigned int currentFeelIdx;

#ifdef DEBUG
    printf("\nbuilding feels tbl ");
#endif
    currentPtr= startAddr;
    currentFeelIdx= 0;

    feelTbl= (char **)malloc(numFeels);

#ifdef DEBUG
    printf("at %x in main mem\n", feelTbl);
#endif

    while (currentFeelIdx < numFeels) {
        feelTbl[currentFeelIdx]= currentPtr;
#ifdef DEBUG
        printf("feel %x at %x\n", currentFeelIdx, currentPtr);
#endif
        while (*currentPtr != 0) {
            currentPtr++;
        }

        currentFeelIdx++;
        currentPtr++;
    }
    return currentPtr;
}

void loadMap(char *filename) {

    byte *currentDungeonPtr;
    byte *feelsPtr;

    byte bytesRead;

    FILE *infile;

#ifdef DEBUG
    printf("load map %s\n\nloading map header\n", filename);
#endif

    infile= fopen(filename, "rb");
    fread(linebuf, 3, 1, infile);
    linebuf[3]= 0;

#ifdef DEBUG
    printf("identifier segment: %s\n", linebuf);
#endif

    if (strcmp(linebuf, "dr0") != 0) {
        printf("?fatal: wrong map file format");
        fclose(infile);
        exit(0);
    }

    fread(&dungeonSize, 2, 1, infile);

    mapdata= (byte *)malloc(dungeonSize);
    currentDungeonPtr= mapdata;

#ifdef DEBUG
    printf("mapdata at %x\n", mapdata);
#endif

    while (!feof(infile)) {
        bytesRead= fread(currentDungeonPtr, 1, BUFSIZE, infile);
        cputc('.');
        currentDungeonPtr+= bytesRead;
    }

#ifdef DEBUG
    printf("\nread mapdata up to %x\n", currentDungeonPtr);
#endif

    dungeonMapWidth= mapdata;
    dungeonMapHeight= mapdata + 1;
    startX= mapdata + 2;
    startY= mapdata + 3;
    dungeon= mapdata + 4;

    seenSpaces= (byte *)malloc((*dungeonMapWidth * *dungeonMapHeight) / 8);
    bzero(seenSpaces, ((*dungeonMapWidth * *dungeonMapHeight) / 8));

#ifdef DEBUG
    printf("map format is %s, dungeon size %x, width %d, height %d.\n", linebuf,
           dungeonSize, *dungeonMapWidth, *dungeonMapHeight);
    printf("startx: %d, starty: %d\n", *startX, *startY);
#endif

    // jump to end of map
    currentDungeonPtr= (dungeon + (*dungeonMapWidth * *dungeonMapHeight * 2));

    // adr= dungeonMapWidth * dungeonMapHeight * 2;

    numFeels= *(currentDungeonPtr + 5);
    *(currentDungeonPtr + 5)= 0; // set string terminator

#ifdef DEBUG
    printf("segment: '%s'\n", currentDungeonPtr);
#endif

    if (strcmp((char *)currentDungeonPtr, "feels") != 0) {
        printf("?fatal: feels segment marker not found");
        fclose(infile);
        exit(0);
    }

#ifdef DEBUG
    printf("%d feels\n", numFeels);
#endif

    feelsPtr= currentDungeonPtr + 6;
    currentDungeonPtr= buildFeelsTable(feelsPtr);

    numOpcs= currentDungeonPtr[4];
    currentDungeonPtr[4]= 0;

#ifdef DEBUG
    printf("segment: %s\n", currentDungeonPtr);
#endif

    if (strcmp(currentDungeonPtr, "opcs") != 0) {
        printf("?fatal: opcs segment marker not found");
        fclose(infile);
        exit(0);
    }

    currentDungeonPtr+= 5; // skip identifier
    opcodesAdr= (opcode *)currentDungeonPtr;

#ifdef DEBUG
    printf("%d opcodes at %x\n", numOpcs, opcodesAdr);
#endif

    lastFeelIndex= 0;
    fclose(infile);

#ifdef DEBUG
    printf("done loading map. press any key.");
    cgetc();
#endif
}

void setupDungeonScreen(void) {
    clrscr();
    textcolor(COLOR_BLUE);
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
    loadMap("map0");
    cgetc();
    textcolor(COLOR_BLACK);
    bordercolor(COLOR_GRAY1);
    bgcolor(COLOR_GRAY2);
    setupDungeonScreen();
    dungeonLoop();
}

void blitmap(byte mapX, byte mapY, byte posX, byte posY) {

    byte drm_dungeonElem;
    register byte *screenPtr; // working pointer to screen
    register byte *bufPtr;    // working pointer to line buffer
    unsigned int startAddr;   // start address in external memory
    unsigned int seenPlacesIdx;
    byte xs; // x and y counter
    byte ys;
    byte screenStride;
    byte mapStride;

    screenStride= screenWidth - mapWindowSize;
    mapStride= *dungeonMapWidth * 2;

    startAddr= (mapX * 2) + (*dungeonMapWidth * (mapY * 2));
    seenPlacesIdx= mapX + (*dungeonMapWidth * mapY);

    screenPtr= SCREEN + (screenWidth * posY) + posX;

    for (ys= 0; ys < mapWindowSize; ++ys) {
        bufPtr= linebuf - 1;
        for (xs= 0; xs < mapWindowSize; ++xs, ++screenPtr) {
            drm_dungeonElem= *++bufPtr;
            ++bufPtr;                    // skip opcode
            if (drm_dungeonElem & 128) { // visible/automapped?
                *screenPtr= signs[drm_dungeonElem & 15];
            } else {
                *screenPtr= 160;
            }
            ++seenPlacesIdx;
        }
        startAddr+= mapStride;
        screenPtr+= screenStride;
    }
}

// clang-format off
#pragma codesize(pop);
// clang-format on
