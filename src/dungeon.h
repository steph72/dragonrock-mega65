#ifndef _dungeonH
#define _dungeonH

#include "types.h"

extern byte *seenMap;

void testMap(void);
void enterDungeonMode(void);
void blitmap(byte mapX, byte mapY, byte posX, byte posY);

#endif