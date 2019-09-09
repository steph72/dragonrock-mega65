#include "encounter.h"
#include <unistd.h>

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
    y= ((2 - row) * 3);

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
    // printf("load sprite %x to idx %x @ %x\n", id,
    // gCurrentSpriteCharacterIndex, addr);
    idxTable[id]= gCurrentSpriteCharacterIndex;
    gCurrentSpriteCharacterIndex+= 8;
    // cgetc();
}

void loadSpriteIfNeeded(byte id) {

    if (idxTable[id] != 255) {
        return;
    }
    loadSprite(id);
}

void loadCharacterSprites(void) {
    byte i;
    for (i= 0; i < partyMemberCount(); ++i) {
        loadSpriteIfNeeded(party[i]->spriteID);
    }
}

byte preEncounter(void) {
    static byte i, j;
    static byte count;
    static monster *aMonster;
    static char *outName;
    static char choice;

    clrscr();
    showCurrentParty(false);
    chlinexy(0, 10, 40);
    gotoxy(0,12);
    for (i= 0; i < MONSTER_ROWS; ++i) {
        count= 0;
        for (j= 0; j < MONSTER_SLOTS; ++j) {
            if (gMonsterRow[i][j]) {
                ++count;
                aMonster= gMonsterRow[i][j];
            }
        }
        if (count == 1) {
            outName= aMonster->def->name;
        } else {
            outName= pluralname(aMonster->def);
        }
        printf("Rank %d: %d %s\n", i + 1, count, outName);
    }

    chlinexy(0, 16, 40);
    gotoxy(0, 18);
    puts("1) Fight      2) Accept Surrender");
    puts("3) Greetings  4) Beg for mercy");
    puts("5) Flee\n");
    cputs(">");
    cursor(1);

    do {
        choice= cgetc();
    } while (choice<'1' || choice>'5');

    cursor(0);

}

encResult doEncounter(void) {

    byte c, i, j;
    monster *aMonster;

    clrscr();
    puts("An encounter!\n");
    sleep(1);
    cg_emptyBuffer();
    
    preEncounter();

    setSplitEnable(1);
    clrscr();
    gotoxy(0, 16);

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