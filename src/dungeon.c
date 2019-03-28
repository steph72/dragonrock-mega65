#include "dungeon.h"
#include "types.h"
#include <c128.h>
#include <conio.h>
#include <em.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOWBYTE(v) ((unsigned char)(v))
#define HIGHBYTE(v) ((unsigned char)(((unsigned int)(v)) >> 8))

#define SCREEN ((unsigned char *)0x400)

byte linebuf[32];
byte dungeonMapWidth;
byte dungeonMapHeight;

// clang-format off
#pragma codesize(push, 300);
// clang-format on

const byte mapWindowSize= 18;
const byte screenWidth= 40;

void test_em(void) {
    register byte i;
    register unsigned int adr;
    struct em_copy emc;

    emc.buf= linebuf;
    emc.count= 32;

    for (adr= 0; adr < 8192; adr+= 32) {
        for (i= 0; i < 32; i++) {
            linebuf[i]= rand() % 255;
        }
        emc.page= 2 + HIGHBYTE(adr);
        emc.offs= LOWBYTE(adr);
        em_copyto(&emc);
    }
}

void loadMap(char *filename) {
    unsigned int adr;
    struct em_copy emc;
    FILE *infile;

    emc.buf= linebuf;
    emc.count= 32;

    infile= fopen(filename, "rb");
    fread(linebuf, 3, 1, infile);
    linebuf[3]= 0;

    if (strcmp(linebuf, "dr0") != 0) {
        printf("?fatal: wrong map file format");
        fclose(infile);
        exit(0);
    }

    fread(&dungeonMapWidth, 1, 1, infile);
    fread(&dungeonMapHeight, 1, 1, infile);
    printf("map format is %s, width %d, height %d.\n", linebuf, dungeonMapWidth,
           dungeonMapHeight);

    adr=0;
    while (!feof(infile)) {
        fread(linebuf, 32, 1, infile);
        emc.page= 2 + HIGHBYTE(adr);
        emc.offs= LOWBYTE(adr);
        em_copyto(&emc);
        adr+= 32;
    }

    printf("read up to address %d\n", adr);
    fclose(infile);
}

void testMap(void) {

    int x;
    loadMap("map0");

    for (x= 0; x < 128; x++) {
        blitmap(x, 0, 3, 3);
        cgetc();
    }
}

void blitmap(byte mapX, byte mapY, byte screenX, byte screenY) {

    register byte *screenPtr; // working pointer to screen
    register byte *bufPtr;    // working pointer to line buffer
    struct em_copy emc;       // external memory control block
    unsigned int startAddr;   // start address in external memory
    register byte xs, ys;     // x and y counter
    byte screenStride;

    screenStride= screenWidth - mapWindowSize;

    startAddr= (mapX*2) + (dungeonMapWidth * mapY);

    emc.buf= linebuf;
    emc.count= 32;

    screenPtr= SCREEN + (screenWidth * screenY) + screenX - 1;

    for (ys= 0; ys < mapWindowSize; ++ys) {
        emc.page= 2 + HIGHBYTE(startAddr);
        emc.offs= LOWBYTE(startAddr);
        em_copyfrom(&emc);
        bufPtr= linebuf - 1;
        for (xs= 0; xs < mapWindowSize; ++xs) {
            *++screenPtr= *++bufPtr;
            ++bufPtr; // skip opcode byte
        }
        startAddr+= (dungeonMapWidth*2);
        screenPtr+= screenStride;
    }
}

// clang-format off
#pragma codesize(pop);
// clang-format on