#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "congui.h"

word testMem() {
    byte *t;
    t= (byte *)malloc(0x100);
    free(t);
    return (word)t;
}