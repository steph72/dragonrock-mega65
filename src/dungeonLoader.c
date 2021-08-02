#include "congui.h"
#include "dungeon.h"
#include "globals.h"
#include "memory.h"
#include <c64.h>
//#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#define DLDEBUG
#endif

#undef DLDEBUG

const himemPtr externalDungeonAddr= ATTIC_DUNGEON_DATA;
himemPtr seenMap;

// clang-format off
#pragma code-name(push, "OVERLAY1");
// clang-format on

void buildFeelsTable(himemPtr *startAddr, dungeonDescriptor *desc,
                     unsigned int numFeels);

void buildDaemonsTable(himemPtr *startAddr, dungeonDescriptor *desc);

// check for correct segment header, get number of elemnts and increase
// himem pointer
unsigned int verifySegment(himemPtr *adr, char *segmentID) {
    long count;
    long countAdr;
#ifdef DLDEBUG
    cg_printf("=== looking for segment %s\n", segmentID);
#endif
    countAdr= (*adr) + (strlen(segmentID));
    count= lpeek(countAdr) + (256 * lpeek(countAdr + 1)); // get count for later
    lpoke(*adr + strlen(segmentID), 0); // end of string marker
    lcopy(*adr, (long)drbuf, 16);
    if (strcmp(segmentID, drbuf) != 0) {
        cg_printf("fatal: marker %s found, %s expected\n", drbuf, segmentID);
        while(1);
    }
    *adr+= strlen(segmentID) + 2;
    return count;
}

dungeonDescriptor *loadMap(char *filename) {

    unsigned int dungeonSize;
    unsigned int numFeels;
    unsigned int numOpcs;
    unsigned int numDaemons;

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
    cg_clrscr();
    cg_printf("load map %s\n\nloading map header\n", filename);
#endif

    mega65_io_enable();
    currentExternalDungeonPtr= externalDungeonAddr;

    
    infile= fopen(filename, "rb");

    if (!infile) {
        cg_fatal("file not found: %s",filename);
    }

    fread(drbuf, 3, 1, infile);
    drbuf[3]= 0;

#ifdef DLDEBUG
    cg_printf("identifier segment: %s\n", drbuf);
#endif

    if (strcmp(drbuf, "dr0") != 0) {
        cg_printf("?fatal: wrong map file format");
        fclose(infile);
        while(1);
    }

    desc= (dungeonDescriptor *)malloc(sizeof(dungeonDescriptor));

#ifdef DLDEBUG
    cg_printf("dungeon descriptor: %x\n", desc);
#endif

    fread(&dungeonSize, 2, 1, infile);

    while (!feof(infile)) {
        ++i;
        bytesRead= fread(drbuf, 1, DRBUFSIZE, infile);
        if (!(i % 8))
            cg_putc('.');
        lcopy((long)drbuf, currentExternalDungeonPtr, DRBUFSIZE);
        currentExternalDungeonPtr+= bytesRead;
    }

    fclose(infile);
    cg_go16bit(0,0);

#ifdef DLDEBUG
    cg_printf("\nread mapdata up to %lx\n", currentExternalDungeonPtr);
#endif

    // get dungeon header from high memory...
    lcopy(externalDungeonAddr, (long)drbuf, 4);

    // ...and fill in descriptions
    desc->dungeonMapWidth= *drbuf;
    desc->dungeonMapHeight= *(drbuf + 1);
    desc->dungeon= externalDungeonAddr + 2;
    desc->mapdata= externalDungeonAddr;

#ifdef DLDEBUG
    cg_printf("dungeon at %lx\n", desc->dungeon);
#endif

    smSize= desc->dungeonMapWidth * desc->dungeonMapHeight;
    lfill(SEENMAP_BASE,255,smSize);
    seenMap = SEENMAP_BASE;


#ifdef DLDEBUG
    cg_printf("dungeon size %x, width %d, height %d.\n", dungeonSize,
           desc->dungeonMapWidth, desc->dungeonMapHeight);
    cg_printf("seen map is at $%lx, size $%x\n", seenMap, smSize);
#endif

    // feels start behind end of dungeon
    currentExternalDungeonPtr=
        externalDungeonAddr + 2 +
        (desc->dungeonMapWidth * desc->dungeonMapHeight * 2);

    // -- FEELS --
    numFeels= verifySegment(&currentExternalDungeonPtr, "feels");
    buildFeelsTable(&currentExternalDungeonPtr, desc, numFeels);

    // -- DAEMS --
    numDaemons= verifySegment(&currentExternalDungeonPtr, "daems");
    desc->numDaemons= numDaemons;
    buildDaemonsTable(&currentExternalDungeonPtr, desc);

    // -- OPCS --
    numOpcs= verifySegment(&currentExternalDungeonPtr, "opcs");
    desc->opcodesAdr= currentExternalDungeonPtr;

#ifdef DLDEBUG
    cg_printf("%d feels\n", numFeels);
    cg_printf("%d opcodes at %lx\n", numOpcs, desc->opcodesAdr);
    cg_printf("%d opcodes remaining.\n", 255 - numOpcs);
    debugPtr= (byte *)malloc(8);
    cg_printf("dungeon: %x-%x (size %x)\n", (int)desc, (int)debugPtr,
           (int)debugPtr - (int)desc);
    free(debugPtr);
    cg_getkey();
#endif

    return desc;
}

void buildDaemonsTable(himemPtr *startAddr, dungeonDescriptor *desc) {

    unsigned int size;
    byte i;
    daemonEntry *table;

    if (desc->numDaemons==0) {
        desc->daemonTbl=NULL;
        return;
    }

    size= (sizeof(daemonEntry) * (desc->numDaemons));
    table= (daemonEntry *)malloc(size);
    lcopy(*startAddr, (long)table, size);
    *startAddr+= size;

    for (i=0;i<desc->numDaemons;++i) {
        cg_printf("daemon %d: %d %d %d %d opc %d",i,table[i].x1,table[i].y1,
        table[i].x2,table[i].y2,table[i].opcodeIndex);
    }

    desc->daemonTbl= table;
}

/**
 * @brief iterate over strings in himem and build pointer table to it
 * 
 * @param startAddr start address in himem 
 * @param desc      dungeon descriptor
 * @param numFeels  number of feels to index (part of map file)
 */
void buildFeelsTable(himemPtr *startAddr, dungeonDescriptor *desc,
                     unsigned int numFeels) {

    himemPtr currentPtr;
    unsigned int currentFeelIdx;

#ifdef DLDEBUG
    cg_printf("\nbuilding feels tbl ");
#endif
    currentPtr= *startAddr;
    currentFeelIdx= 0;
    desc->feelTbl= (himemPtr *)malloc(sizeof(himemPtr) * numFeels);

#ifdef DLDEBUG
    cg_printf("at %x in main mem\n", desc->feelTbl);
    // cgetc();
#endif

    while (currentFeelIdx < numFeels) {
        desc->feelTbl[currentFeelIdx]= currentPtr;
#ifdef DLDEBUG
        cg_putc('.');
        // cg_printf("%d-%lx ", currentFeelIdx, currentPtr);
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
