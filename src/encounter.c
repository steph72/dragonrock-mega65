#include "encounter.h"
#include "utils.h"
#include <unistd.h>

byte iteratorRow= 0;
byte iteratorColumn= 0;

byte gCurrentSpriteCharacterIndex;
byte idxTable[255]; // sprite index cache
static char sfname[8];

int partyAuthorityLevel;
int monsterAuthorityLevel;
byte fightStarted;

char *gEncounterAction_p[]= {"Thrust", "Attack", "Slash",
                             "Parry",  "Cast",   "Shoot"};

char *gEncounterAction[]= {"thrusts", "attacks", "slashes",
                           "parries", "casts",   "shoots"};

// clang-format off
#pragma code-name(push, "OVERLAY3");
// clang-format on

void clearText(void);
void redrawMonsters(void);
void redrawParty(void);

void giveCoins(unsigned int coins) {
    opcode fakeOpcode;
    fakeOpcode.id= 0x8a;
    fakeOpcode.param1= coins % 256;
    fakeOpcode.param2= coins / 256;
    performAddCoinsOpcode(&fakeOpcode);
}

void giveExperience(unsigned int exp) {
    opcode fakeOpcode;
    fakeOpcode.id= 0x8b;
    fakeOpcode.param1= exp % 256;
    fakeOpcode.param2= exp / 256;
    performAddCoinsOpcode(&fakeOpcode);
}

byte isPartyDefeated(void) {
    byte i;
    for (i= 0; i < partyMemberCount(); ++i) {
        if (party[i] && party[i]->status != dead && party[i]->status != down) {
            return false;
        }
    }
    return true;
}

byte xposForMonster(byte numMonsters, byte mPos, byte mWidth) {
    byte width;
    width= 40 / numMonsters;
    return (width * mPos) + (width / 2) - (mWidth / 2);
}

character *chooseRandomCharacter(void) {
    byte i;
    character *ret;

    ret= NULL;
    do {
        i= drand(partyMemberCount());
        if (party[i]->status != dead && party[i]->status != down) {
            ret= party[i];
        }
    } while (ret == NULL);
    return ret;
}

void doMonsterDamage(monster *aMonster, hitResult *result) {
    if (result->critical) {
        result->damage= aMonster->level *
                        (aMonster->def->hitDice + aMonster->def->hitModifier);
    } else {
        result->damage= aMonster->level * (drand(aMonster->def->hitDice) +
                                           aMonster->def->hitModifier);
    }
    return;
}

void doMonsterHit(monster *aMonster, character *opponent, hitResult *result) {
    result->toHit= getArmorClassForCharacter(opponent);
    result->hitRoll= drand(20);
    result->critical= (result->hitRoll == 20);
    result->hitBonus= aMonster->def->hitModifier;
    result->acHit= 20 - (result->hitRoll + result->hitBonus);
    result->success= ((result->acHit <= result->toHit) || result->critical);
    if (result->success) {
        doMonsterDamage(aMonster, result);
    }
}

void doMonsterTurn(byte row, byte column) {

    monster *theMonster;
    character *opponent;
    hitResult hitRes;

    theMonster= gMonsterRow[row][column];
    opponent= chooseRandomCharacter();
    doMonsterHit(theMonster, opponent, &hitRes);

    clearText();
    printf("%s (i %d at %d,%d) attacks %s:\n"
           "hit roll %d+%d (AC %d) vs. AC %d: ",
           theMonster->def->name, theMonster->initiative, row, column,
           opponent->name, hitRes.hitRoll, hitRes.hitBonus, hitRes.acHit,
           hitRes.toHit);
    if (hitRes.success) {
        printf("*HIT*\n");
        printf("%s takes %d points of damage.\n", opponent->name,
               hitRes.damage);
        opponent->aHP-= hitRes.damage;
        if (opponent->aHP <
            (-3 - bonusValueForAttribute(opponent->attributes[aCON]))) {
            printf("%s dies!", opponent->name);
            opponent->status= dead;
            redrawParty();
        } else if (opponent->aHP <= 0) {
            printf("%s goes down!", opponent->name);
            opponent->status= down;
            redrawParty();
        }
    } else {
        printf("*MISS*\n");
    }
    cgetc();
}

void doPartyTurn(byte idx) {
    character *theCharacter;
    encCommand encounterCommand;
    byte encSpell;
    byte encDestinationRank;

    theCharacter= party[idx];
    encounterCommand= theCharacter->currentEncounterCommand;
    encSpell= theCharacter->encSpell;
    encDestinationRank= theCharacter->encDestRank;

    clearText();
    printf("should do %s (i %d): encC %d encS %d encDR %d\n",
           theCharacter->name, theCharacter->initiative, encounterCommand,
           encSpell, encDestinationRank);

    cgetc();
}

void plotSprite(byte x, byte y, char spriteID) {
    byte i, j;
    byte *screenPtr;
    byte charIdx;
    screenPtr= SCREEN + (x - 1 + (y * 40));
    charIdx= idxTable[spriteID] - 1;
    for (i= 0; i < 3; ++i) {
        for (j= 0; j < 3; ++j) {
            *(++screenPtr)= ++charIdx;
        }
        screenPtr+= 37;
    }
}

void eraseSprite(byte x, byte y) {
    byte i, j;
    byte *screenPtr;
    screenPtr= SCREEN + (x - 1 + (y * 40));
    for (i= 0; i < 3; ++i) {
        for (j= 0; j < 3; ++j) {
            *(++screenPtr)= 0xff;
        }
        screenPtr+= 37;
    }
}

void plotMonster(byte row, byte idx) {
    byte x, y;

    x= xposForMonster(gNumMonsters[row], idx, 3);
    y= ((2 - row) * 4);

    plotSprite(x, y, gMonsterRow[row][idx]->def->spriteID);
}

void plotCharacter(byte idx) {
    byte x, y;

    x= xposForMonster(partyMemberCount(), idx, 3);
    y= 13;

    if (party[idx]->status == down) {
        eraseSprite(x, y);
        return;
    }

    if (party[idx]->status == dead) {
        plotSprite(x, y, 0);
        return;
    }

    plotSprite(x, y, party[idx]->spriteID);
}

/**
 * @brief loads a sprite into memory
 *
 * @param id the id of the sprite to load.
 * after loading, the start address of the sprite is
 * added to the sprite index table
 */
void loadSprite(byte id) {
    byte *addr;
    FILE *spritefile;

    sprintf(sfname, "spr%03d", id);
    spritefile= fopen(sfname, "rb");
    cputc('.');
    addr= (byte *)0xf000 + (gCurrentSpriteCharacterIndex * 8);
    // printf("\n%s -> %d @ $%x", sfname, gCurrentSpriteCharacterIndex,
    // addr);
    if (spritefile) {
        fread(addr, 144, 1, spritefile);
        fclose(spritefile);
    } else {
        printf("\n!spritefile %s not found", sfname);
        spritefile= fopen("spr001", "rb");
        fread(addr, 144, 1, spritefile);
        fclose(spritefile);
    }
    idxTable[id]= gCurrentSpriteCharacterIndex;

    /*
        one sprite takes 18 characters (144 bytes),
        3x3 rows = 9 characters x 2 for each state,
        which gives us space for 14 sprites in one
        charset.

    */

    gCurrentSpriteCharacterIndex+= 18;
}

/**
 * @brief adds a sprite to the sprite pool if needed
 *
 * @param id the sprite id
 */
void loadSpriteIfNeeded(byte id) {

    if (idxTable[id] != 255) {
        return;
    }
    loadSprite(id);
}

void clearText(void) {
    cg_clearLower(7);
    gotoxy(0, 17);
}

void showFightOptionStatus(char *status) {
    clearText();
    cputs(status);
    sleep(1);
}

encResult checkSurrender() {
    unsigned int tMonsterAuth;
    tMonsterAuth= monsterAuthorityLevel + drand(3);
    if (tMonsterAuth < partyAuthorityLevel) {
        return encSurrender;
    }
    return encFight;
}

encResult checkMercy() {
    encResult res= encFight;
    if (monsterAuthorityLevel < partyAuthorityLevel) {
        if ((drand(10)) > 7) {
            res= encMercy;
        }
    }
    if (drand(10) > 3) {
        res= encMercy;
    }

    return res;
}

encResult checkGreet() {
    encResult res= encFight;
    int greetChance= 0;
    int greetRoll= 0;

    if (fightStarted) {
        return encFight;
    }

    if (monsterAuthorityLevel + 5 < partyAuthorityLevel) {
        return encGreet;
    }

    if (monsterAuthorityLevel < partyAuthorityLevel) {
        greetChance= 60;
    } else {
        greetChance= 50 - ((monsterAuthorityLevel - partyAuthorityLevel) * 10);
    }

    greetRoll= drand(100);

    printf("greet chance, roll: %d %d", greetChance, greetRoll);
    cgetc();

    if (greetChance > greetRoll) {
        return encGreet;
    } else {
        return encFight;
    }
}

byte _iterateMonsters(monster **currentMonster, byte *row, byte *column) {

    *row= iteratorRow;
    *column= iteratorColumn;

    if (iteratorRow == MONSTER_ROWS) {
        iteratorRow= 0;
        iteratorColumn= 0;
        *currentMonster= NULL;
        return 0;
    }

    *currentMonster= gMonsterRow[iteratorRow][iteratorColumn];

    if (iteratorColumn == MONSTER_SLOTS - 1) {
        iteratorColumn= 0;
        ++iteratorRow;
    } else {
        ++iteratorColumn;
    }

    return 1;
}

byte iterateMonsters(monster **currentMonster, byte *row, byte *column) {
    byte res;

    do {
        res= _iterateMonsters(currentMonster, row, column);
    } while (res == 1 && *currentMonster == NULL);

    return res;
}

encResult preEncounter(void) {

    static byte i, j;
    static byte totalMonsterCount;
    static byte livePartyMembersCount;
    static monster *aMonster;
    static char choice;
    character *aCharacter;

    monsterAuthorityLevel= 0;

    clrscr();
    textcolor(BCOLOR_RED | CATTR_LUMA3);
    chlinexy(0, 10, 40);
    chlinexy(0, 16, 40);
    textcolor(BCOLOR_WHITE | CATTR_LUMA6);
    showCurrentParty(false);
    gotoxy(0, 12);

    // display ranks and compute authority level while we're at it
    totalMonsterCount= 0;
    while (iterateMonsters(&aMonster, &i, &j)) {
        ++totalMonsterCount;
        monsterAuthorityLevel+= aMonster->level;
        monsterAuthorityLevel+= aMonster->def->courageModifier;
    }

    for (i= 0; i < MONSTER_ROWS; ++i) {
        j= gNumMonsters[i];
        if (j) {
            aMonster= gMonsterRow[i][0];
            printf("Rank %d: %d %s(s)\n", i + 1, gNumMonsters[i],
                   aMonster->def->name);
        }
    }

    monsterAuthorityLevel/= totalMonsterCount;

    // calc party authority level
    partyAuthorityLevel= 0;
    livePartyMembersCount= 0;
    for (i= 0; i < partyMemberCount(); ++i) {
        aCharacter= party[i];
        if (aCharacter->status == awake) {
            ++livePartyMembersCount;
            partyAuthorityLevel+= aCharacter->level;
            partyAuthorityLevel+=
                bonusValueForAttribute(aCharacter->attributes[0]);
        }
    }
    partyAuthorityLevel/= livePartyMembersCount;

#ifdef DEBUG
    gotoxy(30, 17);
    printf("(P%d/M%d)", partyAuthorityLevel, monsterAuthorityLevel);
#endif
    gotoxy(0, 18);
    puts("1) Fight      2) Accept Surrender");
    puts("3) Greetings  4) Beg for mercy");
    puts("5) Flee\n");
    cputs(">");
    cursor(1);

    do {
        choice= cgetc();
    } while (choice < '1' || choice > '6');
    cursor(0);

    switch (choice) {
    case '1':
        return encFight; // just fight
        break;

    case '2':
        return checkSurrender();
        break;

    case '3':
        return checkGreet();

    case '4':
        return checkMercy();
        break;

#ifdef DEBUG
    case '6':
        return encWon; // debug win
        break;
#endif

    default:
        break;
    }

    return encFight;
}

void prepareMonsters(void) {

    byte i, j;
    monster *aMonster;

    while (iterateMonsters(&aMonster, &i, &j)) {
        loadSpriteIfNeeded(aMonster->def->spriteID);
        aMonster->initiative=
            (byte)(drand(20)); // todo: honor monster attributes
    }
}

void prepareCharacters(void) {
    byte i;
    for (i= 0; i < partyMemberCount(); ++i) {
        party[i]->initiative=
            (drand(20) + bonusValueForAttribute(party[i]->attributes[3]));
        loadSpriteIfNeeded(party[i]->spriteID);
    }
}

void characterChooseSpell(character *guy) {
    char sp;
    cputs("cast spell nr.? ");
    fgets(drbuf, 10, stdin);
    sp= atoi(drbuf);
    guy->encSpell= sp;
}

void getChoicesForPartyMember(byte idx) {
    character *guy;
    char choice;
    char repeat;
    guy= party[idx];

    do {
        clearText();
        repeat= false;
        cputs(guy->name);
        cputs(":\r\nA)ttack  S)lash  T)hrust  P)arry\r\nF)ire bow  C)ast "
              "spell  "
              "O)ptions\r\n>");
        cursor(1);

        do {
            choice= cgetc();
        } while (strchr("astpco", choice) == NULL);

        switch (choice) {

        case 'a':
            guy->currentEncounterCommand= ec_attack;
            break;

        case 's':
            if (guy->aClass == ct_fighter && guy->level >= 4) {
                guy->currentEncounterCommand= ec_slash;
            } else {
                guy->currentEncounterCommand= ec_attack;
            }
            break;

        case 't':
            if (guy->aClass == ct_fighter && guy->level >= 2) {
                guy->currentEncounterCommand= ec_thrust;
            } else {
                guy->currentEncounterCommand= ec_attack;
            }
            break;

        case 'p':
            guy->currentEncounterCommand= ec_parry;
            break;

        case 'c':
            guy->currentEncounterCommand= ec_magic;
            characterChooseSpell(guy);
            if (guy->encSpell == 0) {
                cputs("spell not known!");
                cg_getkey();
                repeat= true;
            }
            break;

        default:
            break;
        }

    } while (repeat == true);

    cursor(0);
}

void redrawMonsters(void) {
    byte i, j;
    monster *aMonster;
    while (iterateMonsters(&aMonster, &i, &j)) {
        plotMonster(i, j);
    }
}

void redrawParty(void) {
    byte j;
    for (j= 0; j < PARTYSIZE; ++j) {
        if (party[j]) {
            plotCharacter(j);
        }
    }
}

encResult encLoop(void) {

    byte c, i, j;
    monster *aMonster;
    encResult res;
    byte stopEncounter;

    byte gotChoices;

    gCurrentSpriteCharacterIndex= 0;

    bordercolor(BCOLOR_RED | CATTR_LUMA0);
    bgcolor(BCOLOR_BLACK);
    textcolor(BCOLOR_WHITE | CATTR_LUMA6);
    clrscr();

    memset(idxTable, 255, 255);
    loadSprite(0); // tombstone
    prepareMonsters();
    prepareCharacters();

    stopEncounter= false;
    fightStarted= false;

    do {

        setSplitEnable(0);
        cg_emptyBuffer();
        res= preEncounter();

        if (res != encFight) {
            return res;
        }

        fightStarted= true;
        setSplitEnable(1);

        cg_clear();
        gotoxy(0, 17);

        redrawMonsters();
        redrawParty();

        // main encounter loop

        gotChoices= false;
        do {
            for (j= 0; j < PARTYSIZE; ++j) {
                if (party[j]->status == awake) {
                    getChoicesForPartyMember(j);
                }
            }
            clearText();
            for (j= 0; j < PARTYSIZE; ++j) {
                if (party[j]->status == awake) {
                    cputs(party[j]->name);
                    cputs(": ");
                    cputs(
                        gEncounterAction_p[party[j]->currentEncounterCommand]);
                    if (party[j]->encSpell) {
                        printf(" %d ", party[j]->encSpell);
                    }
                    if (party[j]->encDestRank) {
                        printf(" at rank %d", party[j]->encDestRank);
                    }
                    cputs("\r\n");
                }
            }
            cputs("\r\nUse these options (y/n)? ");
            cursor(1);
            do {
                c= cgetc();
            } while (strchr("yn", c) == NULL);
            cursor(0);
            gotChoices= (c == 'y');
        } while (!gotChoices);

        for (c= 20; c != 0; --c) {
            while (iterateMonsters(&aMonster, &i, &j)) {
                if (!isPartyDefeated()) {
                    if (aMonster->initiative == c) {
                        doMonsterTurn(i, j);
                    }
                } else {
                    return encDead;
                }
            }
            for (j= 0; j < PARTYSIZE; ++j) {
                if (party[j] && party[j]->initiative == c) {
                    doPartyTurn(j);
                }
            }
        }

    } while (!stopEncounter);
    return encWon;
}

void takeMoney(void) {
    byte i;
    for (i= 0; i < partyMemberCount(); ++i) {
        party[i]->gold= 0;
    }
    puts("The monsters take all your money.\n");
}

void postKill(byte takeLoot) {

    unsigned int experience= 0;
    unsigned int coins= 0;
    byte newCoins;

    byte i, j;
    monster *aMonster;

    while (iterateMonsters(&aMonster, &i, &j)) {
        newCoins= 0;
        if (aMonster->hp <= 0) {
            experience+= aMonster->def->xpBaseValue;
            newCoins= aMonster->def->xpBaseValue / 10;
        }
        if (takeLoot) {
            if (!(aMonster->def->type & mt_animal)) {
                if (newCoins == 0) {
                    newCoins= 1;
                }
                coins+= newCoins;
            }
        }
    }

    if (coins > 0) {
        giveCoins(coins);
    }

    if (experience > 0) {
        giveExperience(experience);
    }
}

void displayPostfightPrompt(encResult res) {
    switch (res) {

    case encSurrender:
        showFightOptionStatus("The monsters surrender!");
        break;

    case encMercy:
        showFightOptionStatus("The monsters have mercy!");
        break;

    case encWon:
        showFightOptionStatus("The party wins!");
        break;

    case encFled:
        showFightOptionStatus("The party flees!");
        break;

    case encDead:
        showFightOptionStatus("All party members have fallen!");
        break;

    case encGreet:
        showFightOptionStatus("The monsters greet you.");
        break;

    default:
        showFightOptionStatus("?you should not see this");
        break;
    }
}

encResult doEncounter(void) {

    encResult res;

    res= encLoop();
    displayPostfightPrompt(res);

    setSplitEnable(0);

    clrscr();

    if (res == encWon || res == encSurrender) {
        postKill(true);
    }

    if (res == encMercy) {
        takeMoney();
    }

    if (res == encFled) {
        postKill(false);
    }

    if (res != encGreet) {
        cputs("-- key --\n");
        cgetc();
    }

    return res;
}

// clang-format off
#pragma code-name(pop);
// clang-format on
