#include "monster.h"
#include "memory.h"
#include "utils.h"

monster *gMonsterRows[MONSTER_ROWS][MONSTER_SLOTS];

char monsterNameBuf[32];

monsterDef tempDef;

// clang-format off
#pragma code-name(push, "OVERLAY3");
// clang-format on

byte getNumberOfMonsterAttacks(monster *aMonster) {
    byte i;
    byte num = 0;
    monsterDef *def = monsterDefForMonster(aMonster);
    for (i=0;i<4;++i) {
        if (def->aType[i]!=0) {
            num++;
        }
    }
    return num;
}

char *nameForMonsterDef(monsterDef *aDef) {
    lcopy((long)monstersBase + (aDef->namePtr), (long)monsterNameBuf, 32);
    return monsterNameBuf;
}

char *pluralNameForMonsterDef(monsterDef *aDef) {
    lcopy((long)monstersBase + (aDef->pluralnamePtr), (long)monsterNameBuf, 32);
    return monsterNameBuf;
}

char *nameForMonsterID(unsigned int id) {
    return nameForMonsterDef(monsterDefForID(id));
}

char *nameForMonster(monster *aMonster) {
    return nameForMonsterID(aMonster->monsterDefID);
}

char *pluralNameForMonsterID(unsigned int id) {
    monsterDef *aDef= monsterDefForID(id);
    if (aDef->pluralnamePtr) {
        pluralNameForMonsterDef(aDef);
    } else {
        nameForMonsterDef(aDef);
        strcat(monsterNameBuf,"s");
    }
    return monsterNameBuf;
}

// clang-format off
#pragma code-name(pop);
// clang-format on

// -------------------------- common code ----------------------------------

monsterDef *monsterDefForID(unsigned int id) {
    unsigned int i;
    for (i= 0; i < 512; ++i) {
        lcopy((long)monstersBase + 8 + (sizeof(monsterDef) * i),
              (long)&tempDef, sizeof(monsterDef));
        if (tempDef.id == id) {
            return &tempDef;
        }
    }
    return NULL;
}

void _initMonsterRows(byte dealloc) {
    byte i, j;
    for (i= 0; i < MONSTER_ROWS; ++i) {
        for (j= 0; j < MONSTER_SLOTS; ++j) {
            if (dealloc && gMonsterRows[i][j]) {
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
            gMonsterRows[row][i]= aMonster;
            aMonster->row= row;
            aMonster->column= i;
            return;
        }
    }
    cg_fatal("nms");
}

// clear monsters
void clearMonsters(void) { _initMonsterRows(true); }

// create a monster with given ID and level
// (creates standard level if level==0)

monster *createMonster(unsigned int monsterID, byte level) {

    byte i;
    monster *newMonster;
    monsterDef *aDef;

    aDef= monsterDefForID(monsterID);

    if (aDef == NULL) {
        cg_fatal("invm &d",monsterID);
    }

    newMonster= malloc(sizeof(monster));

    if (level == 0) {
        level= aDef->level;
    }

    newMonster->hp= 0;
    newMonster->mp= 0;

    newMonster->monsterDefID= monsterID;
    for (i= 0; i < level; i++) {
        newMonster->hp+= drand(aDef->hpPerLevel) + 1;
        newMonster->mp+= drand(aDef->mpPerLevel) + 1;
    }
    newMonster->level= level;

    return newMonster;
}

// add new monster to row
void addNewMonster(byte monsterID, byte level, byte min, byte max, byte row) {
    byte i;
    byte num;
    monster *theMonster;
    num = max==min ? max : min + (drand(max - min));
    for (i= 0; i < num; ++i) {
        theMonster= createMonster(monsterID, level);
        theMonster->status= awake; /* TODO */
        addMonster(theMonster, row);
    }
}