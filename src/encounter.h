//#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <c64.h>


#include "globals.h"
#include "character.h"
#include "congui.h"
#include "monster.h"

extern monster *gMonsterRows[MONSTER_ROWS][MONSTER_SLOTS];

int performAddCoinsOpcode(opcode *anOpcode);

encResult doEncounter(void);
