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

void plotSprite(byte x, byte y, byte spriteIdx) {
    byte i, j;
    byte *screenPtr;
    byte charIdx;
    screenPtr= SCREEN + (x - 1 + (y * 40));
    charIdx= (spriteIdx * 4) - 1;
    for (i= 0; i < 2; ++i) {
        for (j= 0; j < 2; ++j) {
             *(++screenPtr)= ++charIdx;
        }
        screenPtr+= 38;
    }
}

void plotMonster(byte row, byte idx, byte spriteIdx) {
    byte x, y;

    x= xposForMonster(gNumMonsters[row], idx, 2);
    y= (row * 3);

    plotSprite(x, y, spriteIdx);
}

encResult doEncounter(void) {

    byte c, i, j;

    setSplitEnable(1);
    clrscr();
    gotoxy(0, 16);
    printf("An encounter...\n");
    gotoxy(0, 0);


    // determine number of monsters & do monster initiative rolls

    for (i= 0; i < MONSTER_ROWS; ++i) {
        for (j= 0; j < MONSTER_SLOTS; ++j) {
            if (gMonsterRow[i][j]) {
                gMonsterRow[i][j]->initiative= (byte)(rand() % 20);
                plotMonster(i, j, 0);
            }
        }
    }

    // do party initiative rolls

    for (j= 0; j < PARTYSIZE; ++j) {
        if (party[j]) {
            party[j]->initiative=
                (rand() % 20) + bonusValueForAttribute(party[j]->attributes[3]);
            gotoxy(xposForMonster(partyMemberCount(), j, 3), 12);
            cputs("PPP");
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