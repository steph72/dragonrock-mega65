#include "monster.h"
#include "utils.h"

monster *gMonsterRoster[MROSTER_SIZE];
byte gNumMonstersForRow[MONSTER_SLOTS];

byte gMonsterCount;

// add monster to row
void addMonster(monster *aMonster, byte row) {
    if (gMonsterCount>=MROSTER_SIZE) {
        printf("no more space for monsters");
        exit(0);
    }
    aMonster->row= row;
    aMonster->column= 0;
    gMonsterRoster[gMonsterCount++]= aMonster;
}

// clear monster roster
void clearMonsters(void) {
    byte x;
    gMonsterCount= 0;
    for (x= 0; x < MROSTER_SIZE; x++) {
        if (gMonsterRoster[x]) {
            // free(gMonsterRoster[x]);
            gMonsterRoster[x]= NULL;
        }
    }
}

/*
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
*/

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
        newMonster->hp+= drand(aDef->hpPerLevel)+1;
        newMonster->mp+= drand(aDef->mpPerLevel)+1;
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
        theMonster->status = awake; /* TODO */
        addMonster(theMonster, row);
    }
}