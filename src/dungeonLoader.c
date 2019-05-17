#include "dungeon.h"
#include "types.h"
#include <conio.h>
#include <plus4.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 255

extern dungeonItem *dungeon;
extern unsigned int dungeonSize;
extern byte *seenSpaces;
extern byte *dungeonMapWidth;
extern byte *dungeonMapHeight;
extern byte *startX;
extern byte *startY;
extern opcode *opcodesAdr;      // external address of 1st opcode
extern unsigned char **feelTbl; // pointer to feel addresses

byte numFeels;
byte numOpcs;
byte *mapdata;
byte linebuf[BUFSIZE];

byte *buildFeelsTable(byte *startAddr);

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
    dungeon= (dungeonItem *)(mapdata + 4);

    seenSpaces= (byte *)malloc((*dungeonMapWidth * *dungeonMapHeight) / 8);
    bzero(seenSpaces, ((*dungeonMapWidth * *dungeonMapHeight) / 8));

#ifdef DEBUG
    printf("map format is %s, dungeon size %x, width %d, height %d.\n", linebuf,
           dungeonSize, *dungeonMapWidth, *dungeonMapHeight);
    printf("startx: %d, starty: %d\n", *startX, *startY);
#endif

    // jump to end of map
    currentDungeonPtr=
        (mapdata + 4 + (*dungeonMapWidth * *dungeonMapHeight * 2));

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
    printf("segment: '%s'\n", currentDungeonPtr);
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

    fclose(infile);
}


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