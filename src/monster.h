#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"
#include "congui.h"
#include "globals.h"

#define MONSTER_ROWS  3
#define MONSTER_SLOTS 6
#define MROSTER_SIZE 16

void clearMonsters(void);
void addNewMonster(byte monsterID, byte level, byte num, byte row);
monster* createMonster(byte monsterID, byte level);

char* pluralname(monsterDef *aMonsterDef);

