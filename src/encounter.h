#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <plus4.h>

#include "character.h"
#include "config.h"
#include "congui.h"
#include "types.h"
#include "monster.h"
#include "irq.h"

extern monster *gMonsterRow[MONSTER_ROWS][MONSTER_SLOTS];
extern byte gNumMonsters[MONSTER_ROWS];

encResult doEncounter(void);