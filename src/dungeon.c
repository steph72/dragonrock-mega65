#include "types.h"
#include <c128.h>

#define SCREEN ((unsigned char *)0x400)
#define GARB ((unsigned char *)0x2000)

byte gMap[32*32];

// clang-format off
#pragma codesize(push, 300);
// clang-format on

const byte mapWindowSize= 18;
const byte dungeonMapSize= 32;
const byte screenWidth= 40;

void blitmap(byte mapX, byte mapY, byte screenX, byte screenY) {
    
    byte xs, ys;
    register byte *cs; /* current screen character pointer */
    register byte *cm; /* current map character ptr */
    byte screenStride, dungeonStride;

    screenStride= screenWidth - mapWindowSize;
    dungeonStride= dungeonMapSize - mapWindowSize;
    cs= SCREEN + (screenWidth*screenY) + screenX - 1;
    cm= gMap + (mapY * dungeonMapSize) + mapX - 1;
    for (ys= 0; ys < mapWindowSize; ++ys) {
        for (xs= 0; xs < mapWindowSize; ++xs) {
            *++cs= *++cm;
        }
        cs+= screenStride;
        cm+= dungeonStride;
    }
}

// clang-format off
#pragma codesize(pop);
// clang-format on