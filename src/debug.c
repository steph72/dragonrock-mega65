#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "congui.h"

void testMem() {
    byte *t;
    t= (byte *)malloc(8);
    cg_printf("memtop is $%x\n", t);
    free(t);
}