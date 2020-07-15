#include "monster.h"
#include "utils.h"

monster *gMonsterRows[MONSTER_ROWS][MONSTER_SLOTS];

void _initMonsterRows(byte dealloc) {
    byte i, j;
    for (i= 0; i < MONSTER_ROWS; ++i) {
        for (j= 0; j < MONSTER_SLOTS; ++j) {
            if (dealloc && gMonsterRows[i][j]) {
#ifdef DEBUG
                printf("dealloc monster row %d slot %d:  %x\n",i,j,gMonsterRows[i][j]);
                cgetc();
#endif
                free(gMonsterRows[i][j]);
            }
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
#ifdef DEBUG
            printf("adding monster %x to row %d index %d\n", aMonster, row, i);
#endif
            gMonsterRows[row][i]= aMonster;
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