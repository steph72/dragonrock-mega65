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
byte dungeonMapWidth;
byte dungeonMapHeight;
byte startX;
byte startY;

unsigned int *feelTbl; // pointer to bank 1 feel addresses

// clang-format off
#pragma codesize(push, 300);
// clang-format on

const byte mapWindowSize= 15;
const byte screenWidth= 40;

unsigned int dungeonItemAtPos(byte x, byte y) {
    struct em_copy emc;
    unsigned int exAdr;
    exAdr = (x*2)+(dungeonMapWidth*y*2);
    emc.buf= linebuf;
    emc.count= 2;
    emc.page= 2 + HIGHBYTE(exAdr);
    emc.offs= LOWBYTE(exAdr);
    em_copyfrom(&emc);
    return *linebuf;
}

void dumpMap(void) {
    unsigned int x,y;
    unsigned int c;
    for (x=0;x<dungeonMapWidth;x++) {
        for (y=0;y<24;y++) {
            gotoxy(x,y);
            c=dungeonItemAtPos(x,y);
            cputcxy(x,y,signs[c&7]);
        }
    }
}

char *feelForIndex(byte idx) {
    struct em_copy emc;
    unsigned int exAdr;

    exAdr= feelTbl[idx];

    emc.buf= linebuf;
    emc.count= BUFSIZE;
    emc.page= 2 + HIGHBYTE(exAdr);
    emc.offs= LOWBYTE(exAdr);
    em_copyfrom(&emc);
    return linebuf;
}

void buildFeelsTable(unsigned int startAddr) {
    byte found;
    unsigned int currentBufOffset;
    unsigned int currentExternalAdr;
    unsigned int currentFeelIdx;
    struct em_copy emc;

#ifdef DEBUG
    printf("\nbuilding feels table ");
#endif

    emc.buf= linebuf;
    emc.count= BUFSIZE;
    currentExternalAdr= startAddr;
    currentFeelIdx= 0;

    feelTbl= (unsigned int *)malloc(numFeels);
#ifdef DEBUG
    printf("at %x\n", feelTbl);
#endif

    while (currentFeelIdx < numFeels) {
        feelTbl[currentFeelIdx]= currentExternalAdr;
#ifdef DEBUG
        printf("feel %x at address %x\n", currentFeelIdx, currentExternalAdr);
#endif
        found= false;
        do {
            emc.page= 2 + HIGHBYTE(currentExternalAdr);
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
}

void loadMap(char *filename) {
    unsigned int adr;
    unsigned int feelsAdr;
    struct em_copy emc;
    FILE *infile;

    emc.buf= linebuf;
    emc.count= BUFSIZE;

    infile= fopen(filename, "rb");
    fread(linebuf, 3, 1, infile);
    linebuf[3]= 0;

#ifdef DEBUG
    printf("segment: %s\n", linebuf);
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
    printf("\nloading map...");
#endif

    adr= 0;
    while (!feof(infile)) {
        fread(linebuf, BUFSIZE, 1, infile);
        emc.page= 2 + HIGHBYTE(adr);
        emc.offs= LOWBYTE(adr);
        em_copyto(&emc);
        adr+= BUFSIZE;
    }

#ifdef DEBUG
    printf("\nread map up to address %d\n", adr);
#endif

    adr= dungeonMapWidth * dungeonMapHeight * 2; // jump to end of map
    emc.page= 2 + HIGHBYTE(adr);
    emc.offs= LOWBYTE(adr);
    em_copyfrom(&emc);

    numFeels= linebuf[5];
    linebuf[5]= 0;

#ifdef DEBUG
    printf("segment: %s\n", linebuf);
#endif

    if (strcmp(linebuf, "feels") != 0) {
        printf("?fatal: wrong map file format");
        fclose(infile);
        exit(0);
    }

#ifdef DEBUG
    printf("%d feels\n", numFeels);
#endif

    feelsAdr= adr + 6;
    buildFeelsTable(feelsAdr);
}

void testMap(void) {

    int x;
    loadMap("map0");
    /*
    for (x= 0; x < numFeels; ++x) {
        printf("Feel %d:\n%s\n", x, feelForIndex(x));
    }*/
    dumpMap();
    cgetc();

    for (x= 0; x < 128; x++) {
        blitmap(x, 0, 3, 3);
        cgetc();
    }
}

void blitmap(byte mapX, byte mapY, byte screenX, byte screenY) {

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

    startAddr= (mapX * 2) + (dungeonMapWidth * mapY);

    emc.buf= linebuf;
    emc.count= BUFSIZE;

    screenPtr= SCREEN + (screenWidth * screenY) + screenX - 1;

    for (ys= 0; ys < mapWindowSize; ++ys) {
        emc.page= 2 + HIGHBYTE(startAddr);
        emc.offs= LOWBYTE(startAddr);
        em_copyfrom(&emc);
        bufPtr= linebuf - 1;
        for (xs= 0; xs < mapWindowSize; ++xs, ++screenPtr) {
            drm_dungeonElem= *++bufPtr;
            drm_opcode= *++bufPtr;
            // if (drm_opcode & 128) {
            *screenPtr= signs[drm_dungeonElem & 7];
            //}
        }
        startAddr+= (dungeonMapWidth * 2);
        screenPtr+= screenStride;
    }
}

// clang-format off
#pragma codesize(pop);
// clang-format on