#include <stdio.h>
#include <stdlib.h>

unsigned int drand(unsigned int max) {
    
    unsigned int x;

    do {
        x= rand();
    } while (x >= (RAND_MAX - RAND_MAX % max));

    x %= max;

    return x;
}

unsigned int dmrand(unsigned int min, unsigned int max) {
    return min+(drand(max-min));
}