#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"
#include "config.h"
#include "congui.h"
#include "types.h"

void clearMonsters(void);
void addNewMonster(byte monsterID, byte level, byte row);
monster* createMonster(byte monsterID, byte level);