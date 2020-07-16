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

const himemPtr dungeonBank= 0x050000;
const himemPtr dungeonAddr= 0x000000;

unsigned int dungeonSize;
unsigned int numFeels;
unsigned int numOpcs;
byte *seenMap;

himemPtr externalDungeonAddr;

// clang-format off
#pragma code-name(push, "OVERLAY1");
// clang-format on

void buildFeelsTable(himemPtr *startAddr, dungeonDescriptor *desc);

// check for correct segment header, get number of elemnts and increase himem
// pointer
unsigned int verifySegment(himemPtr *adr, char *segmentID) {
    long count;
    long countAdr;
#ifdef DLDEBUG
    printf("=== looking for segment %s\n", segmentID);
#endif
    countAdr= (*adr) + (strlen(segmentID));
    count= lpeek(countAdr) + (256 * lpeek(countAdr + 1)); // get count for later
    lpoke(*adr + strlen(segmentID), 0); // end of string marker
    lcopy(*adr, (long)drbuf, 16);
    if (strcmp(segmentID, drbuf) != 0) {
        printf("fatal: marker %s found, %s expected\n", drbuf, segmentID);
        exit(0);
    }
    *adr+= strlen(segmentID) + 2;
    return count;
}

dungeonDescriptor *loadMap(char *filename) {

    byte i= 0;

    himemPtr currentExternalDungeonPtr;
    dungeonDescriptor *desc;
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
        lcopy((long)drbuf, currentExternalDungeonPtr, BUFSIZE);
        currentExternalDungeonPtr+= bytesRead;
    }

    fclose(infile);

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
    printf("dungeon size %x, width %d, height %d.\n", dungeonSize,
           desc->dungeonMapWidth, desc->dungeonMapHeight);
    printf("startx: %d, starty: %d\n", desc->startX, desc->startY);
    printf("seen map is at $%x, size $%x\n", seenMap, smSize);
#endif

    // feels start behind end of dungeon
    currentExternalDungeonPtr=
        externalDungeonAddr + 4 +
        (desc->dungeonMapWidth * desc->dungeonMapHeight * 2);

    numFeels= verifySegment(&currentExternalDungeonPtr, "feels");

    // build feels table
    buildFeelsTable(&currentExternalDungeonPtr, desc);

    // -- OPCS --

    numOpcs= verifySegment(&currentExternalDungeonPtr, "opcs");
    desc->opcodesAdr= currentExternalDungeonPtr;

#ifdef DLDEBUG
    printf("%d feels\n", numFeels);
    printf("%d opcodes at %lx\n", numOpcs, desc->opcodesAdr);
    printf("%d opcodes remaining.\n", 255 - numOpcs);
    debugPtr= (byte *)malloc(8);
    printf("dungeon: %x-%x (size %x)\n", (int)desc, (int)debugPtr,
           (int)debugPtr - (int)desc);
    free(debugPtr);
    cgetc();
#endif

    return desc;
}

void buildFeelsTable(himemPtr *startAddr, dungeonDescriptor *desc) {

    himemPtr currentPtr;
    unsigned int currentFeelIdx;

#ifdef DLDEBUG
    printf("\nbuilding feels tbl ");
#endif
    currentPtr= *startAddr;
    currentFeelIdx= 0;
    desc->feelTbl= (himemPtr *)malloc(sizeof(himemPtr) * numFeels);

#ifdef DLDEBUG
    printf("at %x in main mem\n", desc->feelTbl);
    // cgetc();
#endif

    while (currentFeelIdx < numFeels) {
        desc->feelTbl[currentFeelIdx]= currentPtr;
#ifdef DLDEBUG
        printf("%lx ", currentFeelIdx, currentPtr);
#endif
        while (lpeek(currentPtr) != 0) {
            currentPtr++;
        }

        currentFeelIdx++;
        currentPtr++;
    }
    *startAddr= currentPtr;
}

// clang-format off
#pragma code-name(pop);
// clang-format on
