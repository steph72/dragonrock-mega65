#include "encounter.h"

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

void plotMonster(byte row, byte idx) {
    byte charIdx= 0;
    byte monsterSpriteID= 0;
    byte x, y, i, j;
    byte *screenPtr;

    x= xposForMonster(gNumMonsters[row], idx, 3);
    y= 3 + (row * 6);

    screenPtr= SCREEN + (x + y * 40) - 1;
    charIdx= 0x60 + monsterSpriteID - 1;
    for (i= 0; i < 3; ++i) {
        for (j= 0; j < 3; ++j) {
            *(++screenPtr)= 0x5f;
        }
        screenPtr+= 37;
    }
}

encResult doEncounter(void) {

    byte c, i, j;

    clrscr();
    printf("An encounter...\n");

    // determine number of monsters & do monster initiative rolls

    for (i= 0; i < 2; ++i) {
        for (j= 0; j < 5; ++j) {
            if (gMonsterRow[i][j] != NULL) {
                gMonsterRow[i][j]->initiative= (byte)(rand() % 20);
                plotMonster(i, j);
            }
        }
    }

    // do party initiative rolls

    for (j= 0; j < PARTYSIZE; ++j) {
        if (party[j]) {
            party[j]->initiative=
                (rand() % 20) + bonusValueForAttribute(party[j]->attributes[3]);
            gotoxy(xposForMonster(partyMemberCount(), j, 3), 18);
            cputs("PPP");
        }
    }

    cgetc();

    // main encounter loop

    for (c= 20; c != 0; --c) {
        for (i= 0; i < 2; ++i) {
            for (j= 0; j < 5; ++j) {
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

    return encWon;
}