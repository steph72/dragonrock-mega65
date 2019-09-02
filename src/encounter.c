#include "encounter.h"

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

encResult doEncounter(void) {

    byte c, i, j;
    byte numMonsters[2]= {0, 0};

    clrscr();
    printf("An encounter...\n");

    // determine number of monsters & do monster initiative rolls

    for (i= 0; i < 2; ++i) {
        for (j= 0; j < 5; ++j) {
            if (gMonsterRow[i][j] != NULL) {
                gMonsterRow[i][j]->initiative= (byte)(rand() % 20);
                ++numMonsters[i];
            }
        }
    }

    // do party initiative rolls

    for (j= 0; j < PARTYSIZE; ++j) {
        if (party[j]) {
            party[j]->initiative=
                (rand() % 20) + bonusValueForAttribute(party[j]->attributes[3]);
        }
    }

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