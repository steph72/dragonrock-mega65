#include "encounter.h"

byte gCurrentSpriteCharacterIndex;
byte idxTable[255]; // spriteID -> startCharacter mapping

byte xposForMonster(byte numMonsters, byte mPos, byte mWidth) {
    byte width;
    width= 40 / numMonsters;
    return (width * mPos) + (width / 2) - (mWidth / 2);
}

void doMonsterTurn(byte row, byte column) {

    monster *theMonster;
    theMonster= gMonsterRow[row][column];
    printf("doing %s(%d at %d,%d)\n", theMonster->def->name,
           theMonster->initiative, row, column);
    // cgetc();
}

void doPartyTurn(byte idx) {
    character *theCharacter;
    theCharacter= party[idx];
    printf("doing %s(%d at %d)\n", theCharacter->name, theCharacter->initiative,
           idx);
    // cgetc();
}

void plotSprite(byte x, byte y, byte spriteID) {
    byte i, j;
    byte *screenPtr;
    byte charIdx;
    screenPtr= SCREEN + (x - 1 + (y * 40));
    charIdx= idxTable[spriteID] - 1;
    for (i= 0; i < 2; ++i) {
        for (j= 0; j < 2; ++j) {
            *(++screenPtr)= ++charIdx;
        }
        screenPtr+= 38;
    }
}

void plotMonster(byte row, byte idx) {
    byte x, y;

    x= xposForMonster(gNumMonsters[row], idx, 2);
    y= (row * 3);

    plotSprite(x, y, gMonsterRow[row][idx]->def->spriteID);
}

void plotCharacter(byte idx) {
    byte x, y;

    x= xposForMonster(partyMemberCount(), idx, 2);
    y= 12;

    plotSprite(x, y, party[idx]->spriteID);
}

void loadSprite(byte id) {
    byte *addr;
    addr= (byte *)0xf000 + (gCurrentSpriteCharacterIndex * 8);
    printf("load sprite %x to idx %x @ %x\n", id, gCurrentSpriteCharacterIndex, addr);
    idxTable[id]=gCurrentSpriteCharacterIndex;
    gCurrentSpriteCharacterIndex += 8;
    cgetc();
}

void loadSpriteIfNeeded(byte id) {

    if (idxTable[id] != 255) {
        printf("already have sprite id %d\n", id);
        cgetc();
        return;
    }    
    loadSprite(id);
}

void loadCharacterSprites(void) {
    byte i;
    for (i=0;i<partyMemberCount();++i) {
        loadSpriteIfNeeded(party[i]->spriteID);
    }
}

encResult doEncounter(void) {

    byte c, i, j;
    monster *aMonster;

    setSplitEnable(1);
    clrscr();
    gotoxy(0, 16);
    printf("An encounter...%c%c\n", 27, 't');
    gotoxy(0, 0);

    memset(idxTable, 255, 255);

    gCurrentSpriteCharacterIndex= 0;
    loadCharacterSprites();

    // determine number of monsters & do monster initiative rolls

    for (i= 0; i < MONSTER_ROWS; ++i) {
        for (j= 0; j < MONSTER_SLOTS; ++j) {
            if (gMonsterRow[i][j]) {
                aMonster= gMonsterRow[i][j];
                loadSpriteIfNeeded(aMonster->def->spriteID);
                aMonster->initiative= (byte)(rand() % 20);
                plotMonster(i, j);
            }
        }
    }

    // do party initiative rolls

    for (j= 0; j < PARTYSIZE; ++j) {
        if (party[j]) {
            party[j]->initiative=
                (rand() % 20) + bonusValueForAttribute(party[j]->attributes[3]);
            plotCharacter(j);
        }
    }

    cgetc();

    // main encounter loop

    for (c= 20; c != 0; --c) {
        for (i= 0; i < MONSTER_ROWS; ++i) {
            for (j= 0; j < MONSTER_SLOTS; ++j) {
                if (gMonsterRow[i][j] != NULL &&
                    gMonsterRow[i][j]->initiative == c) {
                    doMonsterTurn(i, j);
                }
            }
        }
        for (j= 0; j < PARTYSIZE; ++j) {
            if (party[j] && party[j]->initiative == c) {
                doPartyTurn(j);
            }
        }
    }

    cgetc();

    setSplitEnable(0);

    return encWon;
}