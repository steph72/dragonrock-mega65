#include "globals.h"
#include "memory.h"
#include "types.h"
#include "congui.h"
#include <stdio.h>
#include <stdlib.h>

unsigned int readExt(FILE *inFile, himemPtr addr, byte skipCBMAddressBytes) {

    unsigned int readBytes;
    unsigned int overallRead;
    unsigned long insertPos;

    insertPos= addr;
    overallRead= 0;

    if (skipCBMAddressBytes) {
        fread(drbuf, 1, 2, inFile);
    }

    do {
        readBytes= fread(drbuf, 1, DRBUFSIZE, inFile);
        if (readBytes) {
            overallRead+= readBytes;
            lcopy((long)drbuf, insertPos, readBytes);
            insertPos+= readBytes;
        }
    } while (readBytes);

    return overallRead;
}

unsigned int loadExt(char *filename, himemPtr addr, byte skipCBMAddressBytes) {

    FILE *inFile;
    word readBytes;

    inFile= fopen(filename, "r");
    readBytes= readExt(inFile, addr, skipCBMAddressBytes);
    fclose(inFile);

    if (readBytes==0) {
        cg_fatal("0 bytes from %s",filename);
    }

    return readBytes;
}

unsigned int drand(unsigned int max) {

    unsigned int x;

    do {
        x= rand();
    } while (x >= (RAND_MAX - RAND_MAX % max));

    x%= max;

    return x;
}

unsigned int dmrand(unsigned int min, unsigned int max) {
    return min + (drand(max - min));
}