#include "sprites.h"
#include "types.h"
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void loadSprite(char *name) {
    FILE *spritefile;
    byte *space;

    spritefile= fopen(name, "r");
    space= malloc(64 * 8);
    fread(space, 58 * 8, 1, spritefile);
    lcopy(space, 0x013ffeU, 58 * 8);
    free(space);
}

void initSprites(void) {

    unsigned long spritePointer;
    byte spritePtrLSB;
    byte spritePtrMSB;
    byte i;

    mega65_io_enable();

    POKE(0xD057U, 255); // enable extra wide sprites
    POKE(0xD055U, 255); // enable custom height for all sprites
    POKE(0xD056U, 64);  // sprites are 64 pixels high

    // -------------- testing 64x64 sprites ------------------

    /*
        // set sprite pointers
        for (i= 0; i < 16; i+=2) {
            spritePointer= (0x10000U+i) / 64;
            lpoke(0x13000U, 0);
            lpoke(0x13001U, 0);
        }
        */

    spritePointer= 0x14000U / 64U;
    spritePtrLSB= spritePointer % 256;
    spritePtrMSB= spritePointer / 256;
    /*
    printf("%lx ", spritePointer);
    printf("%x ", spritePtrLSB);
    printf("%x ", spritePtrMSB);
    cgetc();
    */

    for (i= 0; i < 16; i+= 2) {
        lpoke(0x13000U + i, spritePtrLSB);
        lpoke(0x13000U + i + 1, spritePtrMSB);
    }

    // set location of sprite pointers to 0x013000
    // (and set SPRPTR16 for arbitrary sprite locations)
    POKE(0xd06CU, 0x00);
    POKE(0xd06DU, 0x30);
    POKE(0xd06EU, 0x01 | 0x80); // SPRPTR16

    lfill(0x14000U, 0, 0x1000);
    loadSprite("armory.spr");

}

void setSpriteXExpand(byte sprite, byte enabled) {
    byte current;
    current= PEEK(0xd01dU);
    if (enabled) {
        current= current | (1 << sprite);
    } else {
        current= current & (255 - (1 << sprite));
    }
    POKE(0xd01dU, current);
}

void setSpriteYExpand(byte sprite, byte enabled) {
    byte current;
    current= PEEK(0xd017U);
    if (enabled) {
        current= current | (1 << sprite);
    } else {
        current= current & (255 - (1 << sprite));
    }
    POKE(0xd017U, current);
}

void setSpriteEnabled(byte sprite, byte enabled) {
    byte current;
    current= PEEK(0xd015U);
    if (enabled) {
        current= current | (1 << sprite);
    } else {
        current= current & (255 - (1 << sprite));
    }
    POKE(0xd015U, current);
}

void putSprite(byte sprite, int x, byte y) {
    POKE(0xd000U + (2 * sprite), x & 255);
    POKE(0xd001U + (2 * sprite), y);
    if (x > 255) {
        POKE(0xd010, PEEK(0xd010) | (1 << sprite));
    } else {
        POKE(0xd010, PEEK(0xd010) & (255 - (1 << sprite)));
    }
}

void setSpriteColor(byte sprite, byte col) { POKE(0xd027 + sprite, col); }

// TODO: since xemu doesn't do enhanced sprites (as of now),
// postpone encounter graphics until the dev kit arrives... :)

void testSprites() {
    byte i;
    initSprites();
    for (i= 0; i < 6; ++i) {
        setSpriteEnabled(i, 1);
        setSpriteXExpand(i, 0);
        setSpriteYExpand(i, 0);
        setSpriteColor(i, 7);
        putSprite(i, 50 + (64 * (i / 2)), 120 + (64 * (i % 2)));
    }
}