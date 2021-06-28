#include <stdio.h>

unsigned int drand(unsigned int max);
unsigned int dmrand(unsigned int min, unsigned int max);
unsigned int readExt(FILE *inFile, himemPtr addr);
unsigned int loadExt(char *filename, himemPtr addr);
