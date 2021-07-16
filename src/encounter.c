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
#pragma rodata-name (push, "OVERLAY3")
#pragma local-strings (push,on)
// clang-format on

static char *encounterActionNoun[]= {"Wait",
                                     "Thrust",
                                     "Attack",
                                     "Slash",
                                     "Lunge"
                                     "Spell",
                                     "Parry"};

static char *encounterActionVerb[]= {"waits",      "thrusts at", "attacks",
                                     "slashes at", "lunges",     "casts",
                                     "parries"};

void setupCombatScreen(void);
preCombatResult runPreCombat(void);
byte numAliveCharacters();

void showMonsterRowStatus(void);

byte combatStarted;

// get monster count for given row
char getMonsterCountForRow(byte row, byte countOnlyAlive) {
    byte res, i;
    characterStateT status;
    res= 0;
    for (i= 0; i < MONSTER_SLOTS; ++i) {
        if (gMonsterRows[row][i] != NULL) {
            status= gMonsterRows[row][i]->status;
            if (countOnlyAlive) {
                if (status == awake || status == charmed || status == asleep) {
                    res++;
                }
            } else {
                res++;
            }
        }
    }
    return res;
}

// get monster name for given row
char *getMonsterNameForRow(byte row) {
    monster *res;
    byte count= getMonsterCountForRow(row, true);
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
    sprintf(drbuf, "Rank %d - %d %s", row + 1, getMonsterCountForRow(row, true),
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
    cg_textcolor(COLOR_CYAN);
    cg_puts(results[res]);
    sleep(1);
}

void showParty() {
    byte i;
    character *aChar;
    cg_gotoxy(0, 0);
    cg_textcolor(COLOR_GRAY2);
    cg_puts("# Name          Att HP      MP   Status");
    // show party
    for (i= 0; i < partyMemberCount(); ++i) {
        cg_textcolor(COLOR_GREEN);
        aChar= party[i];
        cg_gotoxy(0, 1 + i);
        cg_printf("%d %s", i + 1, aChar->name);
        cg_gotoxy(17, 1 + i);
        cg_printf("%d", getNumberOfAttacks(aChar));
        cg_gotoxy(19, 1 + i);
        sprintf(drbuf, "%d/%d", aChar->aHP, aChar->aMaxHP);
        if (strlen(drbuf) < 5) {
            cg_putc(' ');
        }
        cg_puts(drbuf);
        cg_gotoxy(27, 1 + i);
        sprintf(drbuf, "%d/%d", aChar->aMP, aChar->aMaxMP);
        if (strlen(drbuf) < 5) {
            cg_putc(' ');
        }
        cg_puts(drbuf);
        cg_gotoxy(33, 1 + i);
        if (aChar->status == down) {
            cg_textcolor(COLOR_VIOLET);
            cg_printf("down");
        } else if (aChar->status == dead) {
            cg_textcolor(COLOR_RED);
            cg_printf("dead");
        } else if (aChar->status == asleep) {
            cg_textcolor(COLOR_LIGHTBLUE);
            cg_printf("asleep");
        } else {
            cg_textcolor(COLOR_GREEN);
            cg_printf("ok");
        }
    }
}

void rollInitiative() {
    byte i, j;
    monster *aMonster;
    character *aChar;
    for (i= 0; i < MONSTER_ROWS; ++i) {
        for (j= 0; j < getMonsterCountForRow(i, false); ++j) {
            aMonster= gMonsterRows[i][j];
            aMonster->initiative= 1 + drand(32);
        }
    }
    for (i= 0; i < partyMemberCount(); ++i) {
        aChar= party[i];
        aChar->initiative=
            1 + drand(32) + bonusValueForAttribute(aChar->attributes[aDEX]);
    }
}

character *getRandomCharacter() {
    character *retChar;
    do {
        retChar= party[drand(partyMemberCount())];
    } while (retChar->status == dead || retChar->status == down);
    return retChar;
}

void killParty() {
    byte i;
    for (i= 0; i < partyMemberCount(); ++i) {
        party[i]->status= dead;
    }
    cg_clrscr();
    cg_textcolor(COLOR_LIGHTRED);
    cg_gotoxy(5, 12);
    cg_puts("The party has been defeated...");
    sleep(1);
    cg_textcolor(COLOR_GREEN);
}

void updateMonsterStatus(monster *aMonster) {
    // TODO: sleep/charmed/etc
    if (aMonster->hp <= 0) {
        aMonster->status= dead;
        printf("%s dies.\n", nameForMonster(aMonster));
    }
}

void updateCharacterStatus(character *aChar) {
    if (aChar->aHP == 0 ||
        (aChar->aHP < 0 &&
         aChar->aHP >= bonusValueForAttribute(aChar->attributes[aCON]))) {
        printf("\n%s goes down.\n", aChar->name);
        aChar->status= down;
        return;
    }

    if (aChar->aHP < 0) {
        printf("\n%s dies!\n", aChar->name);
        aChar->status= dead;
    }
}

static char *dtDefault= "";
static char *dtFrost= "cold ";
static char *dtFire= "fire ";

void monsterAttackSingleCharacter(monster *aMonster, character *aChar,
                                  byte attackTypeIndex) {

    byte hitRoll;
    byte isCritical;
    byte isCriticalFailure;
    unsigned int damage;
    signed char acHit;
    signed char destinationAC;
    monsterDef *def;
    attackType type;
    char *damageType;

    def= monsterDefForMonster(aMonster);
    type= def->aType[attackTypeIndex];
    damageType= dtDefault;

    if (type == at_fire) {
        damageType= dtFire;
    } else if (type == at_ice) {
        damageType= dtFrost;
    }
    destinationAC= getArmorClassForCharacter(aChar);
    hitRoll= 1 + drand(20);
    isCritical= (hitRoll == 20);
    isCriticalFailure= (hitRoll == 1);
    hitRoll+= def->hitModifier[attackTypeIndex];
    acHit= 20 - hitRoll;
    printf("%s attacks %s\nand ", nameForMonsterID(aMonster->monsterDefID),
           aChar->name);
    // printf("(roll %d, AC %d, dest AC %d)\n", hitRoll,
    // acHit,destinationAC);

    damage= def->minDmg[attackTypeIndex] +
            drand(def->maxDmg[attackTypeIndex] - def->minDmg[attackTypeIndex]) +
            def->hitModifier[attackTypeIndex];

    if (acHit > destinationAC) {
        if (isCriticalFailure) {
            printf("critically misses,\ngetting hurt for %d points of "
                   "damage.",
                   damage);
            aMonster->hp-= damage;
            updateMonsterStatus(aMonster);
        } else {
            printf("misses.");
        }
    } else {
        if (isCritical) {
            damage*= 2;
            printf("critically hits for\n%d points of %sdamage.", damage,
                   damageType);
        } else {
            printf("hits for %d points of %sdamage.", damage, damageType);
        }
        aChar->aHP-= damage;
        updateCharacterStatus(aChar);
    }
}

encResult doMonsterTurn(monster *aMonster) {
    character *aChar;
    byte i;

    byte numAttacks= getNumberOfMonsterAttacks(aMonster);
    monsterDef *def= monsterDefForMonster(aMonster);

    // TODO: charmed and sleep state
    if (aMonster->status != awake) {
        return encFight;
    }

    for (i= 0; i < numAttacks; ++i) {
        if (numAliveCharacters() == 0) {
            killParty();
            return encDead;
        }
        aChar= getRandomCharacter();
        monsterAttackSingleCharacter(aMonster, aChar, i);
        printf("\n\n");
    }
    return encFight;
}

monster *getRandomMonsterForRow(byte aRow) {
    byte count;
    monster *retMonster;
    count= getMonsterCountForRow(aRow, true);
    if (count == 0) {
        return NULL;
    }
    do {
        retMonster= gMonsterRows[aRow][drand(count)];
    } while (retMonster->status == dead || retMonster->status == down);
    return retMonster;
}

encResult doCharacterTurn(character *aChar) {
    monster *destMonster;
    monsterDef *def;
    destMonster= getRandomMonsterForRow(aChar->encDestination);
    if (!destMonster) {
        return encFled;
    }
    def= monsterDefForMonster(destMonster);
    printf("%s %s %s\n", aChar->name,
           encounterActionVerb[aChar->currentEncounterCommand],
           nameForMonster(destMonster));

    // printf("%x %s:%d (%d)\n", aChar, aChar->name, aChar->initiative,
    //       bonusValueForAttribute(aChar->attributes[aDEX]));

    return encFight;
}

encCommand getCharacterCommandForRangedCombat() {
    byte c;
    cg_puts("1) Shoot   2) Cast    3) Parry\n>");
    do {
        c= cg_getkey() - '0';
    } while (c < 1 || c > 3);
    switch (c) {
    case 1:
        return ec_attack;
        break;

    case 2:
        return ec_spell;

    default:
        return ec_parry;
    }
}

encCommand getCharacterCommand() {
    encCommand choice;
    cg_puts("1) Thrust  2) Attack  3) Slash\n"
            "4) Lunge   5) Cast    6) Parry\n>");
    do {
        choice= cg_getkey() - '0';
    } while (choice < 1 || choice > 6);
    return choice;
}

void queryPartyActions() {
    byte i;
    char choice;
    character *aChar;
    item *weapon;
    byte hasMissile;
    while (1) {
        setupCombatScreen();
        showParty();
        showMonsterRowStatus();
        for (i= 0; i < partyMemberCount(); ++i) {
            aChar= party[i];
            aChar->currentEncounterCommand= 0;
            aChar->encDestination= 0;
            aChar->encSpell= 0;
            if (aChar->status == dead || aChar->status == asleep ||
                aChar->status == charmed || aChar->status == down) {
                continue;
            }
            weapon= getWeapon(aChar);
            hasMissile= (weapon && weapon->type == it_missile);
            cg_block_raw(0, 15, 39, 24, ' ', 0);
            cg_gotoxy(0, 15);
            cg_textcolor(COLOR_LIGHTBLUE);
            cg_puts(aChar->name);
            cg_putc(',');
            cg_textcolor(COLOR_GRAY2);
            cg_puts(" will you");
            if (hasMissile) {
                aChar->currentEncounterCommand=
                    getCharacterCommandForRangedCombat();
            } else {
                aChar->currentEncounterCommand= getCharacterCommand();
            }
            if (aChar->currentEncounterCommand == ec_lunge) {
                aChar->encDestination= 1;
            }
        }
        cg_block_raw(0, 15, 39, 24, ' ', 0);
        cg_gotoxy(0, 15);
        for (i= 0; i < partyMemberCount(); ++i) {
            aChar= party[i];
            if (aChar->currentEncounterCommand) {
                cg_printf("%s: %s\n", aChar->name,
                          encounterActionNoun[aChar->currentEncounterCommand]);
            }
        }
        cg_textcolor(COLOR_LIGHTBLUE);
        cg_puts("Is this ok (y/n)?");
        cg_cursor(1);
        choice= cg_getkey();
        if (choice != 'n') {
            break;
        }
    }
}

byte numAliveCharacters() {
    byte i;
    character *aChar;
    byte alive= 0;
    for (i= 0; i < partyMemberCount(); ++i) {
        aChar= party[i];
        if (aChar->status == awake) {
            ++alive;
        }
    }
    return alive;
}

encResult doFight() {
    signed char currentInitiative;
    monster *aMonster;
    character *aChar;
    encResult res;
    byte i, j;
    combatStarted= true;
    rollInitiative();
    while (1) {
        queryPartyActions();
        for (currentInitiative= 64; currentInitiative > -64;
             --currentInitiative) {
            // check if monster's turn
            for (i= 0; i < MONSTER_ROWS; ++i) {
                for (j= 0; j < MONSTER_SLOTS; ++j) {
                    aMonster= gMonsterRows[i][j];
                    if (aMonster && aMonster->initiative == currentInitiative) {
                        res= doMonsterTurn(aMonster);
                        cg_getkey();

                        if (res != encFight) {
                            return res;
                        }
                    }
                }
            }
            // check if character's turn
            for (i= 0; i < partyMemberCount(); ++i) {
                aChar= party[i];
                if (aChar->initiative == currentInitiative) {
                    res= doCharacterTurn(aChar);
                    cg_getkey();

                    if (res != encFight) {
                        return res;
                    }
                }
            }
        }
        cg_getkey();
        res= runPreCombat();
        if (res != preCombatResultBeginFight) {
            signalPreCombatResult(res);
        }

        switch (res) {
        case preCombatResultFleeSuccess:
            return encFled;
            break;

        case preCombatResultMercy:
            return encMercy;
            break;

        case preCombatResultSurrender:
            return encSurrender;
            break;

        default:
            break;
        }

        if (numAliveCharacters() == 0) {
            killParty();
            return encDead;
        }
    }
}

encResult doEncounter() {
    preCombatResult res;
    combatStarted= false;
    res= runPreCombat();
    signalPreCombatResult(res);
    if (res == preCombatResultBeginFight || res == preCombatResultFleeFailure) {
        return doFight();
    } else if (res == preCombatResultFleeSuccess) {
        return encFled;
    }
    clearMonsters();
    return encWon;
}

// ------------------ screen config ---------------------

void setupCombatScreen(void) {
    cg_bgcolor(0);
    cg_textcolor(4);
    cg_bordercolor(2);
    cg_clrscr();
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
    cg_gotoxy(0, 22);
    cg_printf("pCourage, mCourage: %d, %d ", pc, mc);
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
    case 'g':
        res= checkGreet();
        break;

    case 't': // threaten
        res= checkThreaten();
        break;

    case 'b': // beg for mercy
        res= checkBegForMercy();
        break;

    case 'f': // fight
        res= preCombatResultBeginFight;
        break;

    case 'r': // flee
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

void showMonsterRowStatus(void) {
    byte i;
    cg_textcolor(COLOR_PURPLE);
    cg_gotoxy(0, 9);
    cg_puts("You are facing:");
    cg_textcolor(COLOR_LIGHTRED);
    for (i= 0; i < MONSTER_ROWS; i++) {
        cg_gotoxy(0, 10 + i);
        if (getMonsterCountForRow(MONSTER_ROWS - i - 1, true)) {
            cg_puts(statusLineForRow(MONSTER_ROWS - i - 1));
        }
    }
}

preCombatResult runPreCombat(void) {
    byte choice;

    setupCombatScreen();
    showParty();
    showMonsterRowStatus();

    cg_gotoxy(0, 20);
    cg_textcolor(COLOR_GREEN);
    cg_puts("g)reet  t)hreaten  b)eg for mercy\nf)ight  r)un away");
    cg_textcolor(COLOR_ORANGE);
    do {
        cg_gotoxy(0, 22);
        cg_puts("Your choice:");
        cg_cursor(1);
        choice= cg_getkey();
        cg_cursor(0);
    } while (strchr("gtbfr", choice) == NULL);

    return preCombatResultForChoice(choice);
}

// clang-format off
#pragma code-name(pop);
#pragma rodata-name (pop)
#pragma local-strings (pop)
// clang-format on
