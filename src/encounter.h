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
extern byte gNumMonstersForRow[MONSTER_ROWS];
extern monster *gMonsterRoster[MROSTER_SIZE];
extern byte gMonsterCount;



/**
 * @brief iterates over the monster positions in a battle
 * 
 * @param currentMonster    monster for current row and column 
 * or NULL if last monster was iterated
 * 
 * @param row               the current battle row
 * @param column            current battle column
 * @return byte             true if there's more coming, false if finished
 */
byte iterateMonsters(monster **currentMonster, byte *row, byte *column);

byte performAddCoinsOpcode(opcode *anOpcode);
encResult doEncounter(void);