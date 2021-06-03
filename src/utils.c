#include "globals.h"
#include "types.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

unsigned int loadExt(char *filename, himemPtr addr) {

    FILE *inFile;
    unsigned int readBytes;
    unsigned int overallRead;
    unsigned long insertPos;

    insertPos = addr;
    overallRead = 0;
    inFile= fopen(filename, "r");

    do {
        readBytes = fread(textbuf,1,TEXTBUF_SIZE,inFile);
        if (readBytes) {
            overallRead += readBytes;
            lcopy((long)textbuf,insertPos,readBytes);
            insertPos+=readBytes;
        }
    } while (readBytes);

    return overallRead;

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