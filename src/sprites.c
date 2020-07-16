#include "sprites.h"
#include "types.h"

void initSprites(void) {
    mega65_io_enable();
    POKE(0xD057U, 255); // enable extra wide sprites
    POKE(0xD055U, 255); // enable custom height for all sprites
    POKE(0xD056U, 64);  // sprites are 64 pixels high
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

// TODO: since xemu doesn't do enhanced sprites (as of now), 
// postpone encounter graphics until the dev kit arrives... :) 

void testSprites() {
    initSprites();
    setSpriteEnabled(0, 1);
    setSpriteEnabled(1, 1);
    putSprite(0, 255, 100);
    putSprite(1, 280, 130);
}