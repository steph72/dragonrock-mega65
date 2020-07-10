#include "dungeon.h"
#include "globals.h"
#include "memory.h"
#include <c64.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#define DLDEBUG
#endif

// #undef DLDEBUG

typedef long himemPtr;

const himemPtr dungeonBank= 0x050000;
const himemPtr dungeonAddr= 0x000000;

unsigned int dungeonSize;
byte numFeels;
byte numOpcs;
byte *seenMap;

himemPtr externalDungeonAddr;

// clang-format off
#pragma code-name(push, "OVERLAY1");
// clang-format on

himemPtr buildFeelsTable(himemPtr startAddr, dungeonDescriptor *desc);

dungeonDescriptor *loadMap(char *filename) {

    byte i= 0;

    himemPtr currentExternalDungeonPtr;
    himemPtr externalFeelsPtr;

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
    clrscr();
    printf("load map %s\n\nloading map header\n", filename);
#endif

    mega65_io_enable();
    externalDungeonAddr= dungeonBank | dungeonAddr;
    currentExternalDungeonPtr= externalDungeonAddr;

    infile= fopen(filename, "rb");

    if (!infile) {
        cputs("file not found: ");
        cputs(filename);
        exit(0);
    }

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

    while (!feof(infile)) {
        ++i;
        bytesRead= fread(drbuf, 1, BUFSIZE, infile);
        if (!(i % 8))
            cputc('.');
        printf("%lx ", currentExternalDungeonPtr);
        lcopy((long)drbuf, currentExternalDungeonPtr, BUFSIZE);
        currentExternalDungeonPtr+= bytesRead;
    }

#ifdef DLDEBUG
    printf("\nread mapdata up to %lx\n", currentExternalDungeonPtr);
#endif

    // get dungeon header from high memory...
    lcopy(externalDungeonAddr, (long)drbuf, 4);

    // ...and fill in descriptions
    desc->dungeonMapWidth= *drbuf;
    desc->dungeonMapHeight= *(drbuf + 1);
    desc->startX= *(drbuf + 2);
    desc->startY= *(drbuf + 3);
    desc->dungeon= externalDungeonAddr + 4;
    desc->mapdata= externalDungeonAddr;

#ifdef DLDEBUG
    printf("dungeon at %lx\n", desc->dungeon);
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

    // feels start behind end of dungeon
    currentExternalDungeonPtr=
        externalDungeonAddr + 4 +
        (desc->dungeonMapWidth * desc->dungeonMapHeight * 2);
    printf("feels should start at %lx", currentExternalDungeonPtr);
    lcopy(currentExternalDungeonPtr, (long)drbuf, 16);
    numFeels= *(drbuf + 5);
    *(drbuf + 5)= 0; // set string terminator for checking segment name

#ifdef DLDEBUG
    printf("segment: '%s'\n", drbuf);
#endif

    if (strcmp((char *)drbuf, "feels") != 0) {
        printf("?seg marker");
        fclose(infile);
        exit(0);
    }

#ifdef DLDEBUG
    printf("%d feels\n", numFeels);
#endif

    // feelsPtr= currentDungeonPtr + 6;
    // currentDungeonPtr= buildFeelsTable(feelsPtr, desc);
    cgetc();
    externalFeelsPtr= currentExternalDungeonPtr + 6;
    currentExternalDungeonPtr= buildFeelsTable(externalFeelsPtr, desc);

    // -- HIER WEITER --- 

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
    cgetc();
#endif

    return desc;
}

himemPtr buildFeelsTable(long startAddr, dungeonDescriptor *desc) {
    
    himemPtr currentPtr;
    unsigned int currentFeelIdx;

#ifdef DLDEBUG
    printf("\nbuilding feels tbl ");
#endif
    currentPtr= startAddr;
    currentFeelIdx= 0;
    desc->feelTbl= (long *)malloc(sizeof(long) * numFeels);

#ifdef DLDEBUG
    printf("at %x in main mem\n", desc->feelTbl);
    cgetc();
#endif

    while (currentFeelIdx < numFeels) {
        desc->feelTbl[currentFeelIdx]= currentPtr;
#ifdef DLDEBUG
        printf("feel %x at %lx\n", currentFeelIdx, currentPtr);
        // printf("feel %x at %x: %s\n", currentFeelIdx, currentPtr,
        // currentPtr);
#endif
        while (lpeek(currentPtr) != 0) {
            currentPtr++;
        }

        currentFeelIdx++;
        currentPtr++;
    }
    return currentPtr;
}

// clang-format off
#pragma code-name(pop);
// clang-format on
