//#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"
#include "congui.h"
#include "globals.h"

#define MONSTER_ROWS  3
#define MONSTER_SLOTS 6
#define MROSTER_SIZE 16

#define monsterDefForMonster(A) monsterDefForID(A->monsterDefID)
#define pluralNameForMonster(A) pluralNameForMonsterID(A->monsterDefID)

monsterDef *monsterDefForID(unsigned int id);
char *nameForMonsterDef(monsterDef *aDef);
char *nameForMonsterID(unsigned int id);
char *pluralNameForMonsterID(unsigned int id);
char *nameForMonster(monster *aMonster);
byte getNumberOfMonsterAttacks(monster *aMonster);

void clearMonsters(void);
void addNewMonster(byte monsterID, byte level, byte min, byte max, byte row);
monster* createMonster(unsigned int monsterID, byte level);

void initMonsterRows();

char* pluralname(monsterDef *aMonsterDef);

