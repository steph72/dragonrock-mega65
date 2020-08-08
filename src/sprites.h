#include "types.h"
#include "memory.h"

void testSprites();

void initSprites(void);
void setSpriteEnabled(byte sprite, byte enabled);
void putSprite(byte sprite, int x, byte y);
void setSpriteColor(byte sprite, byte col);

void setSpriteYExpand(byte sprite, byte enabled);
void setSpriteXExpand(byte sprite, byte enabled);

void loadSprite(char *name, byte spriteNo, byte width, byte height);