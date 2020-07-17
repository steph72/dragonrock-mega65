#include "encounter.h"
#include "globals.h"
#include "spell.h"
#include "types.h"
#include "utils.h"
#include "congui.h"

#include <c64.h>
#include <unistd.h>

char *encounterActionNoun[]= {"Wait",  "Thrust", "Attack", "Slash",
                              "Parry", "Cast",   "Shoot"};

char *encounterActionVerb[]= {"waits",   "thrusts", "attacks", "slashes",
                              "parries", "casts",   "shoots"};

// clang-format off
#pragma code-name(push, "OVERLAY3");
// clang-format on

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
        gotoxy(0, 21+i);
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
    cputs("Encounter...");
    test();
    cgetc();
    clearMonsters();
    test();
    cgetc();
    return encWon;
}

// clang-format off
#pragma code-name(pop);
// clang-format on
