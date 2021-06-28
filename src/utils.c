#include "globals.h"
#include "memory.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>

unsigned int readExt(FILE *inFile, himemPtr addr) {

    unsigned int readBytes;
    unsigned int overallRead;
    unsigned long insertPos;

    insertPos= addr;
    overallRead= 0;

    do {
        readBytes= fread(textbuf, 1, TEXTBUF_SIZE, inFile);
        if (readBytes) {
            overallRead+= readBytes;
            lcopy((long)textbuf, insertPos, readBytes);
            insertPos+= readBytes;
        }
    } while (readBytes);

    fclose(inFile);

    return overallRead;
}

unsigned int loadExt(char *filename, himemPtr addr) {

    FILE *inFile;
    inFile= fopen(filename, "r");

    return readExt(inFile, addr);
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