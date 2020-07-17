#include "monster.h"
#include "utils.h"

monster *gMonsterRows[MONSTER_ROWS][MONSTER_SLOTS];
monster *gMonsterRoster[MONSTER_SLOTS * MONSTER_ROWS];

void _initMonsterRoster(byte dealloc) {
    byte i;
    for (i= 0; i < MONSTER_ROWS * MONSTER_SLOTS; i++) {
        if (dealloc && gMonsterRoster[i]) {
            free(gMonsterRoster[i]);
        }
        gMonsterRoster[i]= NULL;
    }
}

void _addMonsterToRoster(monster *aMonster) {
    byte i;
    for (i= 0; i < MONSTER_ROWS * MONSTER_SLOTS; i++) {
        if (!gMonsterRoster[i]) {
            gMonsterRoster[i]= aMonster;
            return;
        }
    }
    printf("?no more space in monster roster");
}

void _initMonsterRows(byte dealloc) {
    byte i, j;
    _initMonsterRoster(dealloc);
    for (i= 0; i < MONSTER_ROWS; ++i) {
        for (j= 0; j < MONSTER_SLOTS; ++j) {
            gMonsterRows[i][j]= NULL;
        }
    }
}

void initMonsterRows() { _initMonsterRows(false); }

// add monster to row
void addMonster(monster *aMonster, byte row) {
    byte i;
    for (i= 0; i < MONSTER_SLOTS; ++i) {
        if (gMonsterRows[row][i] == NULL) {
            gMonsterRows[row][i]= aMonster;
            aMonster->row = row;
            aMonster->column = i;
            _addMonsterToRoster(aMonster);
            return;
        }
    }
    printf("?no more space for monsters");
    exit(0);
}

// clear monster roster
void clearMonsters(void) { _initMonsterRows(true); }

// create a monster with given ID and level
// (creates standard level if level==0)

monster *createMonster(byte monsterID, byte level) {

    byte i;
    monster *newMonster;
    monsterDef *aDef;

    aDef= NULL;

    for (i= 0; i < 255; ++i) {
        if (gMonsters[i].id == monsterID) {
            aDef= &gMonsters[i];
            break;
        }
        if (gMonsters[i].id == 255) { // stop if end of monster list reached
            break;
        }
    }

    if (aDef == NULL) {
        printf("?invalid monster ID %d", monsterID);
        exit(0);
    }

    newMonster= malloc(sizeof(monster));

    if (level == 0) {
        level= aDef->level;
    }

    newMonster->hp= 0;
    newMonster->mp= 0;

    newMonster->def= aDef;
    for (i= 0; i < level; i++) {
        newMonster->hp+= drand(aDef->hpPerLevel) + 1;
        newMonster->mp+= drand(aDef->mpPerLevel) + 1;
    }
    newMonster->level= level;

    return newMonster;
}

// add new monster to row
void addNewMonster(byte monsterID, byte level, byte num, byte row) {
    byte i;
    monster *theMonster;
    for (i= 0; i < num; ++i) {
        theMonster= createMonster(monsterID, level);
        theMonster->status= awake; /* TODO */
        addMonster(theMonster, row);
    }
}