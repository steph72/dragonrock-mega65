#include "dungeon.h"
#include "types.h"
#include <c128.h>
#include <conio.h>
#include <em.h>
#include <stdlib.h>

#define LOWBYTE(v) ((unsigned char)(v))
#define HIGHBYTE(v) ((unsigned char)(((unsigned int)(v)) >> 8))

#define SCREEN ((unsigned char *)0x400)

byte gMap[32 * 32];
byte lineBuf[32];

// clang-format off
#pragma codesize(push, 300);
// clang-format on

const byte mapWindowSize= 18;
const byte dungeonMapWidth= 32;
const byte screenWidth= 40;

void test_em(void) {
    register byte i;
    byte x, y;
    register unsigned int adr;
    struct em_copy emc;

    emc.buf= lineBuf;
    emc.count= 32;

    for (adr= 0; adr < 8192; adr+= 32) {
        for (i= 0; i < 32; i++) {
            lineBuf[i]= HIGHBYTE(adr);
        }
        emc.page= 2 + HIGHBYTE(adr);
        emc.offs= LOWBYTE(adr);
        em_copyto(&emc);
    }

    for (x= 0; x < 128; x++) {
        blitmap(x, x, 3, 3);
        cgetc();
    }
}

void blitmap(byte mapX, byte mapY, byte screenX, byte screenY) {

    register byte *screenPtr; // working pointer to screen
    register byte *bufPtr;    // working pointer to line buffer
    struct em_copy emc;       // external memory control block
    unsigned int startAddr;   // start address in external memory
    register byte xs, ys;     // x and y counter
    byte screenStride;

    screenStride= screenWidth - mapWindowSize;

    startAddr= mapX + (dungeonMapWidth * mapY);

    emc.buf= lineBuf;
    emc.count= 32;

    screenPtr= SCREEN + (screenWidth * screenY) + screenX - 1;

    for (ys= 0; ys < mapWindowSize; ++ys) {
        emc.page= 2 + HIGHBYTE(startAddr);
        emc.offs= LOWBYTE(startAddr);
        em_copyfrom(&emc);
        bufPtr= lineBuf - 1;
        for (xs= 0; xs < mapWindowSize; ++xs) {
            *++screenPtr= *++bufPtr;
        }
        startAddr+= dungeonMapWidth;
        screenPtr+= screenStride;
    }
}

// clang-format off
#pragma codesize(pop);
// clang-format on