#include "c65.h"

char testVIC4(void) {
    mega65_io_enable();
    POKE(53248U,1);
    POKE(53248U+256,0);
    if (PEEK(53248U)!=1) {
        return false;
    }
    return true;
}

