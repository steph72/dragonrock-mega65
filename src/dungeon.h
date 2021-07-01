#ifndef _dungeonH
#define _dungeonH

#include "types.h"

void testMap(void);
void enterDungeonMode(byte reInitMap);
void blitmap(byte mapX, byte mapY, byte posX, byte posY);
void printFeelForIndex(byte idx);

#endif