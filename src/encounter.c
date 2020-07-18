#include "encounter.h"
#include "congui.h"
#include "globals.h"
#include "spell.h"
#include "types.h"
#include "utils.h"

#include <c64.h>
#include <unistd.h>

char *encounterActionNoun[]= {"Wait",  "Thrust", "Attack", "Slash",
                              "Parry", "Cast",   "Shoot"};

char *encounterActionVerb[]= {"waits",   "thrusts", "attacks", "slashes",
                              "parries", "casts",   "shoots"};

// clang-format off
#pragma code-name(push, "OVERLAY3");
// clang-format on

void showPartyOptions(void);

// get monster name for given row
char *getMonsterNameForRow(byte row) {
    monster *res;
    res= gMonsterRows[row][0];
    if (res) {
        return res->def->name;
    }
    return "---";
}

// get monster count for given row
char getMonsterCountForRow(byte row) {
    byte res, i;
    res= 0;
    for (i= 0; i < MONSTER_SLOTS; ++i) {
        if (gMonsterRows[row][i] != NULL) {
            res++;
        }
    }
    return res;
}

// remove a monster at a given row, column
void removeMonster(byte row, byte column) {
    byte i;
    monster *m;

    m= gMonsterRows[row][column];
    m->row= 255;
    m->column= 255;

    for (i= column; i < MONSTER_SLOTS - 1; ++i) {
        gMonsterRows[row][i]= gMonsterRows[row][i + 1];
        gMonsterRows[row][i]->column= i;
    }

    gMonsterRows[row][MONSTER_SLOTS]= NULL;
}

// remove a monster from combat
// (does NOT delete or free the monster, just removes it from active encounter)
void removeMonsterFromCombat(monster *aMonster) {
    removeMonster(aMonster->row, aMonster->column);
}

char *statusLineForRow(byte row) {
    sprintf(drbuf, "Rank %d: %d %s(s)", row + 1, getMonsterCountForRow(row),
            getMonsterNameForRow(row));
    return drbuf;
}

void preCombatScreen() {
    byte i;
    cg_titlec(COLOR_RED, COLOR_GRAY3, 20, "An encounter!");
    for (i= 0; i < MONSTER_ROWS; i++) {
        gotoxy(0, 21 + i);
        cputs(statusLineForRow(i));
    }
}

void test() {
    byte i;
    preCombatScreen();
}

encResult doEncounter() {
    bordercolor(COLOR_BLACK);
    bgcolor(COLOR_BLACK);
    textcolor(COLOR_RED);
    showPartyOptions();
    cgetc();
    test();
    cgetc();
    clearMonsters();
    test();
    cgetc();
    return encWon;
}

// ------------------ screen config ---------------------

void showPartyOptions(void) {
    byte i, j;
    character *aChar;

    // setup screen
    clrscr();
    bgcolor(COLOR_BLACK);
    for (i= 29; i < 40; ++i) {
        for (j= 7; j < 24; ++j) {
            *(SCREEN + (j * 40) + i)= 160;
            *(COLOR_RAM + (j * 40) + i)= COLOR_GRAY2;
        }
    }
    for (i= 0; i < 40; ++i) {
        *(SCREEN + i)= 160;
        *(COLOR_RAM + i)= COLOR_RED;
        for (j= 0; j < 3; ++j) {
            if (getMonsterCountForRow(j)) {
                *(SCREEN + ((24 - j) * 40) + i)= 160;
                *((COLOR_RAM) + ((24 - j) * 40) + i)= COLOR_GREEN;
            }
        }
        for (j= 1; j < 7; ++j) {
            *(SCREEN + (j * 40) + i)= 160;
            *((COLOR_RAM) + (j * 40) + i)= COLOR_YELLOW;
        }
    }
    gotoxy(0, 0);
    textcolor(COLOR_RED);
    revers(1);
    cputs("# ---Name--- Attacks  Hit pts  -Magics-");
    textcolor(COLOR_GRAY3);
    gotoxy(4, 7);
    cputs(" dead ");
    textcolor(COLOR_YELLOW);
    gotoxy(11, 7);
    cputs(" okay ");
    textcolor(COLOR_BLUE);
    gotoxy(18, 7);
    cputs(" sleep ");

    // show party
    for (i= 0; i < partyMemberCount(); ++i) {
        aChar= party[i];
        textcolor(COLOR_YELLOW);
        gotoxy(0, 1 + i);
        printf("%d %s", i + 1, aChar->name);
        gotoxy(16, 1 + i);
        printf("%d", getNumberOfAttacks(aChar));
        gotoxy(23, 1 + i);
        sprintf(drbuf, "%d/%d", aChar->aHP, aChar->aMaxHP);
        if (strlen(drbuf) < 5) {
            cputc(' ');
        }
        cputs(drbuf);
        gotoxy(32, 1 + i);
        sprintf(drbuf, "%d/%d", aChar->aMP, aChar->aMaxMP);
        if (strlen(drbuf) < 5) {
            cputc(' ');
        }
        cputs(drbuf);
    }
}

// clang-format off
#pragma code-name(pop);
// clang-format on
