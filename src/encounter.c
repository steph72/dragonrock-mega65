#include "encounter.h"
#include "congui.h"
#include "globals.h"
#include "memory.h"
#include "spell.h"
#include "types.h"
#include "utils.h"

#include <c64.h>
#include <unistd.h>

// clang-format off
#pragma code-name(push, "OVERLAY3");
// clang-format on

static char *encounterActionNoun[]= {"Wait",  "Thrust", "Attack", "Slash",
                                     "Parry", "Cast",   "Shoot"};

static char *encounterActionVerb[]= {"waits",   "thrusts", "attacks", "slashes",
                                     "parries", "casts",   "shoots"};

void setupCombatScreen(void);
byte runPreCombat(void);

byte combatStarted;

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

// get monster name for given row
char *getMonsterNameForRow(byte row) {
    monster *res;
    byte count= getMonsterCountForRow(row);
    res= gMonsterRows[row][0];
    if (res) {
        if (count > 1) {
            return pluralNameForMonsterID(res->monsterDefID);
        } else {
            return nameForMonsterID(res->monsterDefID);
        }
    }
    return "---";
}

byte getMonsterCount() {
    byte i, j, res;
    res= 0;
    for (i= 0; i < MONSTER_ROWS; ++i) {
        for (j= 0; j < MONSTER_SLOTS; ++j) {
            if (gMonsterRows[i][j]) {
                ++res;
            }
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
// (does NOT delete or free the monster, just removes it from active
// encounter)
void removeMonsterFromCombat(monster *aMonster) {
    removeMonster(aMonster->row, aMonster->column);
}

char *statusLineForRow(byte row) {
    sprintf(drbuf, "Rank %d - %d %s", row + 1, getMonsterCountForRow(row),
            getMonsterNameForRow(row));
    return drbuf;
}

void signalPreCombatResult(preCombatResult res) {
    char *results[]= {"The monsters greet you.",
                      "The monsters surrender.",
                      "The monsters take your money.",
                      "You don't get away.",
                      "You manage to flee",
                      "A fight begins!",
                      "No response"};

    setupCombatScreen();
    textcolor(COLOR_CYAN);
    puts(results[res]);
    sleep(1);
}

encResult doFight() {
    //TODO
    return encWon;
}

encResult doEncounter() {
    preCombatResult res;
    combatStarted= false;
    res= runPreCombat();
    signalPreCombatResult(res);
    if (res==preCombatResultBeginFight) {
        return doFight();
    } else if (res==preCombatResultFleeSuccess) {
        return encFled;
    }
    clearMonsters();
    return encWon;
}

// ------------------ screen config ---------------------

void setupCombatScreen(void) {
    bgcolor(0);
    textcolor(4);
    bordercolor(2);
    clrscr();
}

// ------------------ courage handling  ---------------------

unsigned int mc, pc;


unsigned int partyCourage() {
    byte i;
    unsigned int courage= 0;
    character *aChar;

    for (i= 0; i < partyMemberCount(); ++i) {
        aChar= party[i];
        courage+= (aChar->level) * 100;
        courage+= bonusValueForAttribute(aChar->attributes[aCHR]) *
                  25; // add/remove CHR bonus
    }

    return courage;
}

unsigned int monstersCourage() {
    byte i, j;
    unsigned int courage= 0;
    monster *aMonster;
    monsterDef *def;
    for (i= 0; i < MONSTER_ROWS; ++i) {
        for (j= 0; j < MONSTER_SLOTS; ++j) {
            aMonster= gMonsterRows[i][j];
            if (aMonster) {
                def= monsterDefForID(aMonster->monsterDefID);
                courage+= (aMonster->level) * 110;
                courage+= (def->courageModifier) * 25;
            }
        }
    }
    if (combatStarted) {
        courage+= 100;
    }
    return courage;
}

void updateCourage() {
    mc= monstersCourage() + drand(50 * getMonsterCount());
    pc= partyCourage();
           gotoxy(0, 22);
        printf("pCourage, mCourage: %d, %d ", pc, mc);
}

preCombatResult checkGreet() {
    if (combatStarted) {
        return preCombatResultNoResponse;
    }
    if (mc < pc) {
        return preCombatResultGreet;
    }
    return preCombatResultBeginFight;
}

preCombatResult checkThreaten() {
    if (mc < pc) {
        return preCombatResultSurrender;
    }
    return combatStarted ? preCombatResultNoResponse
                         : preCombatResultBeginFight;
}

preCombatResult checkBegForMercy() {
    if (abs(mc - pc) < 10) {
        return preCombatResultMercy;
    }
    return combatStarted ? preCombatResultNoResponse
                         : preCombatResultBeginFight;
}

preCombatResult preCombatResultForChoice(byte choice) {

    preCombatResult res;
    updateCourage();

    switch (choice) {
    case 1:
        return checkGreet();
        break;

    case 2: // threaten
        return checkThreaten();
        break;

    case 3: // beg for mercy
        return checkBegForMercy();
        break;

    case 4: // fight
        return combatStarted ? preCombatResultNoResponse
                             : preCombatResultBeginFight;
        break;

    case 5: // flee
        if (drand(100) > 50) {
            res= preCombatResultFleeSuccess;
        } else {
            res= preCombatResultFleeFailure;
        }
        break;

    default:
        res= preCombatResultNoResponse;
        break;
    }

    return res;
}

byte runPreCombat(void) {
    byte i, j;
    byte choice;
    preCombatResult res;
    character *aChar;

    setupCombatScreen();
    gotoxy(0, 0);
    textcolor(COLOR_GRAY2);
    cputs("# ---Name--- Attacks  Hit pts  -Magics-");

    // show party
    for (i= 0; i < partyMemberCount(); ++i) {
        textcolor(aChar->aHP > 0 ? COLOR_GREEN : COLOR_RED);
        aChar= party[i];
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

    // show enemies
    textcolor(COLOR_ORANGE);
    gotoxy(0, 11);
    puts("You are facing:");
    textcolor(COLOR_LIGHTRED);
    for (i= 0; i < MONSTER_ROWS; i++) {
        gotoxy(0, 12 + i);
        if (getMonsterCountForRow(MONSTER_ROWS - i - 1)) {
            cputs(statusLineForRow(MONSTER_ROWS - i - 1));
        }
    }
    gotoxy(0, 16);
    textcolor(COLOR_GREEN);
    puts("1) greet  2) threaten  3) beg for mercy");
    puts("4) fight  5) attempt to flee\n");
    textcolor(COLOR_ORANGE);
    do {
        gotoxy(0, 18);
        cputs("Your choice:");
        cursor(1);
        choice= cg_getkey() - '0';
        cursor(0);
        res= preCombatResultForChoice(choice);
        signalPreCombatResult(res);
        // tf(" %d ==>%d  ", choice, res);
    } while (1); // (choice == 0 || choice > 5);

    return preCombatResultForChoice(choice);

}

// clang-format off
#pragma code-name(pop);
// clang-format on
