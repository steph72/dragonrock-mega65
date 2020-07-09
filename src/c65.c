#include "c65.h"

char testVIC3(void) {
    setFastVIC3Mode();
    POKE(53248U,1);
    POKE(53248U+256,0);
    if (PEEK(53248U)!=1) {
        return false;
    }
    return true;
}

void setFastVIC3Mode() {
    byte *vicmode= 0xd031U;
    POKE(0xd02fU, 165);
    POKE(0xd02fU, 150);
    *vicmode|= 64;
}

