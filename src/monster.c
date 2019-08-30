#include "monster.h"

monster *gMonsterRow[2][5];

// add monster to row
void addMonster(monster *aMonster, byte row) {
    byte i;
    byte added;
    added = false;
    for (i=0;i<5;++i) {
        if (gMonsterRow[row][i]==NULL) {
            gMonsterRow[row][i] = aMonster;
            added=true;
            printf("added monster %d to row %d pos %d\n",aMonster->def->id,row,i);
            cgetc();
            break;
        }
    }
    if (!added) {
        printf("could not add monster %d to row %d",aMonster->def->id,row);
        exit(0);
    }
} 



// clear monster roster
void clearMonsters(void) {
    byte x, y;
    for (x= 0; x < 2; x++) {
        for (y= 0; y < 5; y++) {
            if (gMonsterRow[x][y]) {
                free(gMonsterRow[x][y]);
                gMonsterRow[x][y]= NULL;
            }
        }
    }
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
            printf("found monster id %d",i);
            aDef= &gMonsters[i];
            break;
        }
        if (gMonsters[i].id==255) { // stop if end of monster list reached
            break;
        }
    }

    if (aDef == NULL) {
        printf("?invalid monster ID %d",monsterID);
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
void addNewMonster(byte monsterID, byte level, byte row) {
    monster *theMonster;
    theMonster = createMonster(monsterID,level);
    addMonster(theMonster,row);
}