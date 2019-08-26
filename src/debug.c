#include <stdio.h>
#include <stdlib.h>
#include "types.h"

void testMem() {
    byte *t;
    t= (byte *)malloc(8);
    printf("memtop is $%x\n", t);
    free(t);
}