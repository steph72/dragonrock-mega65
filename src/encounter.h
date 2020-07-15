#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <c64.h>


#include "globals.h"
#include "character.h"
#include "congui.h"
#include "monster.h"
#include "irq.h"

extern byte gNumMonstersForRow[MONSTER_ROWS];
extern monster *gMonsterRoster[MROSTER_SIZE];
extern byte gMonsterCount;

byte performAddCoinsOpcode(opcode *anOpcode);

encResult doEncounter(void);
