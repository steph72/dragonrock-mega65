#include "monster.h"

monster *gMonsterRow[MONSTER_ROWS][MONSTER_SLOTS];
byte gNumMonsters[MONSTER_ROWS];

// add monster to row
void addMonster(monster *aMonster, byte row) {
    byte i;
    byte added;
    added= false;
    for (i= 0; i < MONSTER_SLOTS; ++i) {
        if (gMonsterRow[row][i] == NULL) {
            gMonsterRow[row][i]= aMonster;
            ++gNumMonsters[row];
            added= true;
            break;
        }
    }

    printf("\n");
    if (!added) {
        printf("err addm %d row %d", aMonster->def->id, row);
        exit(0);
    }
}

// clear monster roster
void clearMonsters(void) {
    byte x, y;
    for (x= 0; x < MONSTER_ROWS; x++) {
        gNumMonsters[x]= 0;
        for (y= 0; y < MONSTER_SLOTS; y++) {
            if (gMonsterRow[x][y]) {
                free(gMonsterRow[x][y]);
                gMonsterRow[x][y]= NULL;
            }
        }
    }
}

char* pluralname(monsterDef *aMonsterDef) {
    byte i;
    if (aMonsterDef->pluralname) {
        return aMonsterDef->pluralname;
    }
    strcpy(drbuf,aMonsterDef->name);
    i=strlen(drbuf);
    drbuf[i]='s';
    drbuf[i+1]=0;
    return drbuf;
}

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

    newMonster->def= aDef;
    newMonster->hp= aDef->hpPerLevel * level;
    newMonster->mp= aDef->mpPerLevel * level;

    return newMonster;
}

// add new monster to row
void addNewMonster(byte monsterID, byte level, byte num, byte row) {
    byte i;
    monster *theMonster;
    for (i= 0; i < num; ++i) {
        theMonster= createMonster(monsterID, level);
        addMonster(theMonster, row);
    }
}