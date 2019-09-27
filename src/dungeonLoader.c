#include "dungeon.h"
#include "types.h"
#include <conio.h>
#include <plus4.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#define DLDEBUG
#endif

#undef DLDEBUG

unsigned int dungeonSize;
byte numFeels;
byte numOpcs;
byte *mapdata;
byte *seenMap;


// clang-format off
#pragma code-name(push, "OVERLAY1");
// clang-format on


byte *buildFeelsTable(byte *startAddr, dungeonDescriptor *desc);

dungeonDescriptor *loadMap(char *filename) {

    byte i= 0;

    dungeonDescriptor *desc;

    byte *currentDungeonPtr;
    byte *feelsPtr;
    int smSize;

#ifdef DLDEBUG
    byte *debugPtr;
#endif

    byte bytesRead;

    FILE *infile;

#ifdef DLDEBUG
    printf("load map %s\n\nloading map header\n", filename);
#endif

    infile= fopen(filename, "rb");
    fread(drbuf, 3, 1, infile);
    drbuf[3]= 0;

#ifdef DLDEBUG
    printf("identifier segment: %s\n", drbuf);
#endif

    if (strcmp(drbuf, "dr0") != 0) {
        printf("?fatal: wrong map file format");
        fclose(infile);
        exit(0);
    }

    desc= (dungeonDescriptor *)malloc(sizeof(dungeonDescriptor));

#ifdef DLDEBUG
    printf("dungeon descriptor: %x\n", desc);
#endif

    fread(&dungeonSize, 2, 1, infile);

    mapdata= (byte *)malloc(dungeonSize);
    currentDungeonPtr= mapdata;

#ifdef DLDEBUG
    printf("mapdata at %x\n", mapdata);
#endif


    while (!feof(infile)) {
        ++i;
        bytesRead= fread(currentDungeonPtr, 1, BUFSIZE, infile);
        if (!(i%8))
            cputc('.');
        currentDungeonPtr+= bytesRead;
    }

#ifdef DLDEBUG
    printf("\nread mapdata up to %x\n", currentDungeonPtr);
#endif

    desc->dungeonMapWidth= *mapdata;
    desc->dungeonMapHeight= *(mapdata + 1);
    desc->startX= *(mapdata + 2);
    desc->startY= *(mapdata + 3);
    desc->dungeon= (dungeonItem *)(mapdata + 4);
    desc->mapdata = mapdata;

#ifdef DLDEBUG
    printf("dungeon at %x\n", desc->dungeon);
#endif

    smSize= desc->dungeonMapWidth * desc->dungeonMapHeight;
    seenMap= (byte *)malloc(smSize);
    memset(seenMap, 255, smSize);

#ifdef DLDEBUG
    printf("map format is %s, dungeon size %x, width %d, height %d.\n", drbuf,
           dungeonSize, desc->dungeonMapWidth, desc->dungeonMapHeight);
    printf("startx: %d, starty: %d\n", desc->startX, desc->startY);
    printf("seen map is at $%x, size $%x\n", seenMap, smSize);
#endif

    // jump to end of mapW
    currentDungeonPtr=
        (mapdata + 4 + (desc->dungeonMapWidth * desc->dungeonMapHeight * 2));

    numFeels= *(currentDungeonPtr + 5);
    *(currentDungeonPtr + 5)= 0; // set string terminator

#ifdef DLDEBUG
    printf("segment: '%s'\n", currentDungeonPtr);
#endif

    if (strcmp((char *)currentDungeonPtr, "feels") != 0) {
        printf("?seg marker");
        fclose(infile);
        exit(0);
    }

#ifdef DLDEBUG
    printf("%d feels\n", numFeels);
#endif

    feelsPtr= currentDungeonPtr + 6;
    currentDungeonPtr= buildFeelsTable(feelsPtr, desc);

    numOpcs= currentDungeonPtr[4];
    currentDungeonPtr[4]= 0;

#ifdef DLDEBUG
    printf("segment: '%s'\n", currentDungeonPtr);
#endif

    if (strcmp(currentDungeonPtr, "opcs") != 0) {
        printf("?opcs marker");
        fclose(infile);
        exit(0);
    }

    currentDungeonPtr+= 5; // skip identifier and count
    desc->opcodesAdr= (opcode *)currentDungeonPtr;

#ifdef DLDEBUG
    printf("%d opcodes at %x\n", numOpcs, desc->opcodesAdr);
    printf("%d opcodes remaining.\n", 255 - numOpcs);
#endif

    fclose(infile);

#ifdef DLDEBUG
    debugPtr= (byte *)malloc(8);
    printf("dungeon: %x-%x (size %x)\n", (int)desc, (int)debugPtr,
           (int)debugPtr - (int)desc);
    free(debugPtr);
#endif

    return desc;
}

byte *buildFeelsTable(byte *startAddr, dungeonDescriptor *desc) {
    byte *currentPtr;
    unsigned int currentFeelIdx;

#ifdef DLDEBUG
    printf("\nbuilding feels tbl ");
#endif
    currentPtr= startAddr;
    currentFeelIdx= 0;
    desc->feelTbl= (char **)malloc(2 * numFeels);

#ifdef DLDEBUG
    printf("at %x in main mem\n", desc->feelTbl);
#endif

    while (currentFeelIdx < numFeels) {
        desc->feelTbl[currentFeelIdx]= currentPtr;
#ifdef DLDEBUG
        printf("feel %x at %x: %s\n", currentFeelIdx, currentPtr,currentPtr);
#endif
        while (*currentPtr != 0) {
            currentPtr++;
        }

        currentFeelIdx++;
        currentPtr++;
    }
    return currentPtr;
}

// clang-format on
#pragma code-name(pop);
// clang-format off
