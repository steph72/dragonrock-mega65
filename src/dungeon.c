#include "dungeon.h"
#include "types.h"
#include <c128.h>
#include <conio.h>
#include <em.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 128

#define LOWBYTE(v) ((unsigned char)(v))
#define HIGHBYTE(v) ((unsigned char)(((unsigned int)(v)) >> 8))
#define SCREEN ((unsigned char *)0x400)

char signs[]= {
    0x60, // empty space
    0x5a, // diamond
    0x5d, // vertical door
    0x40, // horizontal door
    0xe0, // filled space
};

byte linebuf[BUFSIZE];
byte numFeels;
byte numOpcs;
byte dungeonMapWidth;
byte dungeonMapHeight;
byte startX;
byte startY;

byte lastFeelIndex;

byte currentX; // current coordinates inside map window
byte currentY;

byte offsetX;
byte offsetY;

unsigned int *feelTbl;   // pointer to bank 1 feel addresses
unsigned int opcodesAdr; // external address of 1st opcode

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
    unsigned int *inbuf;
    struct em_copy emc;
    unsigned int exAdr;

    inbuf= (unsigned int *)linebuf;
    exAdr= (x * 2) + (dungeonMapWidth * y * 2);
    emc.buf= linebuf;
    emc.count= 2;
    emc.page= HIGHBYTE(exAdr);
    emc.offs= LOWBYTE(exAdr);
    em_copyfrom(&emc);
    *(linebuf)|= 128;
    em_copyto(&emc);
    return *inbuf;
}

void redrawMap() { blitmap(offsetX, offsetY, screenX, screenY); }

void ensureSaneOffset() {

    if (currentX > mapWindowSize - scrollMargin - 1) {
        if (currentX + mapWindowSize < dungeonMapWidth) {
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
        if (currentY + mapWindowSize < dungeonMapHeight) {
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

void plotDungeonItem(unsigned int dItem, byte x, byte y) {

    register byte *screenPtr; // working pointer to screen

    screenPtr= SCREEN + (screenWidth * screenY) + screenX - 1;

    screenPtr+= x;
    screenPtr+= (y * screenWidth);

    *screenPtr= signs[dItem & 7];
}

void plotPlayer(byte x, byte y) {
    register byte *screenPtr; // working pointer to screen

    screenPtr= SCREEN + (screenWidth * screenY) + screenX - 1;

    screenPtr+= x;
    screenPtr+= (y * screenWidth);

    *screenPtr= 0;
}

void performDisplayFeelOpcode(opcode *anOpcode) {
    byte i;
    byte feelIndex;

    feelIndex = anOpcode->param1;
    if (feelIndex==lastFeelIndex) {
        return;
    }

    for (i=20;i<24;++i) {
        cclearxy(0,i,40);
    }
    gotoxy(0, 20);
    cprintf(feelForIndex(anOpcode->param1));
    lastFeelIndex = feelIndex;
}

void performOpcode(opcode *anOpcode) {

    if (anOpcode->id == 0x01) {
        performDisplayFeelOpcode(anOpcode);
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
    currentX= startX - 1;
    currentY= startY - 1;
    offsetX= 0;
    offsetY= 0;
    mposX= 0;
    mposY= 0;

    if (currentX > mapWindowSize) {
        offsetX= currentX - (mapWindowSize / 2);
        currentX= mapWindowSize / 2;
    }

    if (currentY > mapWindowSize) {
        offsetY= currentY - (mapWindowSize / 2) - 1;
        currentY= mapWindowSize / 2;
    }

    redrawMap();

    // dloop starts here

    do {

        ensureSaneOffset();

        for (xdiff= -1; xdiff <= 1; xdiff++) {
            for (ydiff= -1; ydiff <= 1; ydiff++) {
                mposX= currentX + offsetX + xdiff;
                mposY= currentY + offsetY + ydiff;
                if (mposX >= 0 && mposY >= 0 && mposX < dungeonMapWidth &&
                    mposY < dungeonMapHeight) {
                    dItem= dungeonItemAtPos(mposX, mposY);
                    if (xdiff == 0 & ydiff == 0) {
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
        if (mposX < 0 || mposY < 0 || mposX > dungeonMapWidth ||
            mposY > dungeonMapHeight) {
            // nope, reset position
            currentX= oldX;
            currentY= oldY;
        } else {
            // what is here?
            dItem= dungeonItemAtPos(mposX, mposY);
            if ((dItem & 7) == 4) { // solid wall?
                // can't go there
                currentX= oldX;
                currentY= oldY;
            } else {

            }
        }

    } while (!quit);
}

void dumpMap(void) {
    unsigned int x, y;
    unsigned int c;
    for (x= 0; x < dungeonMapWidth; x++) {
        for (y= 0; y < 24; y++) {
            gotoxy(x, y);
            c= dungeonItemAtPos(x, y);
            cputcxy(x, y, signs[c & 7]);
        }
    }
}

opcode *opcodeForIndex(byte idx) {
    struct em_copy emc;
    unsigned int exAdr;

    exAdr= opcodesAdr + (8 * idx);

    emc.buf= linebuf;
    emc.count= BUFSIZE;
    emc.page= HIGHBYTE(exAdr);
    emc.offs= LOWBYTE(exAdr);
    em_copyfrom(&emc);
    return (opcode *)linebuf;
}

char *feelForIndex(byte idx) {
    struct em_copy emc;
    unsigned int exAdr;

    exAdr= feelTbl[idx];

    emc.buf= linebuf;
    emc.count= BUFSIZE;
    emc.page= HIGHBYTE(exAdr);
    emc.offs= LOWBYTE(exAdr);
    em_copyfrom(&emc);
    return linebuf;
}

unsigned int buildFeelsTable(unsigned int startAddr) {
    byte found;
    unsigned int currentBufOffset;
    unsigned int currentExternalAdr;
    unsigned int currentFeelIdx;
    struct em_copy emc;

#ifdef DEBUG
    printf("\nbuilding feels tbl ");
#endif

    emc.buf= linebuf;
    emc.count= BUFSIZE;
    currentExternalAdr= startAddr;
    currentFeelIdx= 0;

    feelTbl= (unsigned int *)malloc(numFeels);
#ifdef DEBUG
    printf("at %x in main mem\n", feelTbl);
#endif

    while (currentFeelIdx < numFeels) {
        feelTbl[currentFeelIdx]= currentExternalAdr;
#ifdef DEBUG
        printf("feel %x at %x\n", currentFeelIdx, currentExternalAdr);
#endif
        found= false;
        do {
            emc.page= HIGHBYTE(currentExternalAdr);
            emc.offs= LOWBYTE(currentExternalAdr);
            em_copyfrom(&emc);
            currentBufOffset= 0;
            while (currentBufOffset < BUFSIZE &&
                   *(linebuf + currentBufOffset) != 0) {

                ++currentBufOffset;
            }
            found= *(linebuf + currentBufOffset) == 0;
            if (found) {
                ++currentBufOffset;
                ++currentFeelIdx;
            }
            currentExternalAdr+= currentBufOffset;
        } while (found == false);
    }
    return currentExternalAdr;
}

void loadMap(char *filename) {

    unsigned int adr;
    unsigned int feelsAdr;

    struct em_copy emc;
    FILE *infile;

#ifdef DEBUG
    printf("load map %s\n\nloading map header\n", filename);
#endif

    emc.buf= linebuf;
    emc.count= BUFSIZE;

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

    fread(&dungeonMapWidth, 1, 1, infile);
    fread(&dungeonMapHeight, 1, 1, infile);
    fread(&startX, 1, 1, infile);
    fread(&startY, 1, 1, infile);

#ifdef DEBUG
    printf("map format is %s, width %d, height %d.\n", linebuf, dungeonMapWidth,
           dungeonMapHeight);
    printf("startx: %d, starty: %d\n", startX, startY);
    printf("\nloading mapdata...");
#endif

    adr= 0;
    while (!feof(infile)) {
        fread(linebuf, BUFSIZE, 1, infile);
        emc.page= HIGHBYTE(adr);
        emc.offs= LOWBYTE(adr);
        em_copyto(&emc);
        adr+= BUFSIZE;
    }

#ifdef DEBUG
    printf("\nread map up to ext address %x\n", adr);
#endif

    adr= dungeonMapWidth * dungeonMapHeight * 2; // jump to end of map
    emc.page= HIGHBYTE(adr);
    emc.offs= LOWBYTE(adr);
    em_copyfrom(&emc);

    numFeels= linebuf[5];
    linebuf[5]= 0;

#ifdef DEBUG
    printf("segment: %s\n", linebuf);
#endif

    if (strcmp(linebuf, "feels") != 0) {
        printf("?fatal: feels segment marker not found");
        fclose(infile);
        exit(0);
    }

#ifdef DEBUG
    printf("%d feels\n", numFeels);
#endif

    feelsAdr= adr + 6;
    opcodesAdr= buildFeelsTable(feelsAdr);

    emc.page= HIGHBYTE(opcodesAdr);
    emc.offs= LOWBYTE(opcodesAdr);
    em_copyfrom(&emc);

    numOpcs= linebuf[4];
    linebuf[4]= 0;

#ifdef DEBUG
    printf("segment: %s\n", linebuf);
#endif

    if (strcmp(linebuf, "opcs") != 0) {
        printf("?fatal: opcs segment marker not found");
        fclose(infile);
        exit(0);
    }

    opcodesAdr+= 5; // skip identifier

#ifdef DEBUG
    printf("%d opcodes at external memory %x\n", numOpcs, opcodesAdr);
#endif

    lastFeelIndex = 0;
    fclose(infile);

#ifdef DEBUG
    printf("done loading map. press any key.");
    cgetc();
#endif
}

void testMap(void) {
    clrscr();
    cprintf("**mapdebug**\r\n");
    loadMap("map0");
    clrscr();
    dungeonLoop();
}

void blitmap(byte mapX, byte mapY, byte posX, byte posY) {

    register byte drm_opcode;
    register byte drm_dungeonElem;

    register byte *screenPtr; // working pointer to screen
    register byte *bufPtr;    // working pointer to line buffer
    struct em_copy emc;       // external memory control block
    unsigned int startAddr;   // start address in external memory
    register byte xs;         // x and y counter
    byte ys;
    byte screenStride;

    screenStride= screenWidth - mapWindowSize;

    startAddr= (mapX * 2) + (dungeonMapWidth * (mapY * 2));

    emc.buf= linebuf;
    emc.count= BUFSIZE;

    screenPtr= SCREEN + (screenWidth * posY) + posX - 1;

    for (ys= 0; ys < mapWindowSize; ++ys) {
        emc.page= HIGHBYTE(startAddr);
        emc.offs= LOWBYTE(startAddr);
        em_copyfrom(&emc);
        bufPtr= linebuf - 1;
        for (xs= 0; xs < mapWindowSize; ++xs, ++screenPtr) {
            drm_dungeonElem= *++bufPtr;
            drm_opcode= *++bufPtr;
            if (drm_dungeonElem & 128) {
                *screenPtr= signs[drm_dungeonElem & 7];
            } else {
                *screenPtr= 160;
            }
        }
        startAddr+= (dungeonMapWidth * 2);
        screenPtr+= screenStride;
    }
}

// clang-format off
#pragma codesize(pop);
// clang-format on