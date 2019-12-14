#include "encounter.h"
#include "utils.h"
#include <unistd.h>

byte gCurrentSpriteCharacterIndex;
byte idxTable[255]; // sprite index cache

static char sfname[8];
static char hasDoneTurn[6];

int partyAuthorityLevel;
int monsterAuthorityLevel;
byte fightStarted;

char *gEncounterAction_p[]= {"Wait",  "Thrust", "Attack", "Slash",
                             "Parry", "Cast",   "Shoot"};

char *gEncounterAction[]= {"waits",   "thrusts", "attacks", "slashes",
                           "parries", "casts",   "shoots"};

/*
    ==========
    prototypes
    ==========
*/

void clearText(void);
void redrawMonsters(void);
void redrawParty(void);
void plotMonster(monster *aMonster);
void eraseMonster(monster *aMonster);
void plotCharacter(byte idx, byte variant);

/* code starts here */

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

byte areMonstersDefeated(void) {
    byte i;
    monster *aMonster;
    byte defeated;
    for (i= 0; i < gMonsterCount; i++) {
        aMonster= gMonsterRoster[i];
        defeated= (aMonster->status == dead || aMonster->status == surrendered);
        if (!defeated) {
            return false;
        }
    }
    return true;
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

void characterChooseSpell(character *guy) {
    char sp;
    cputs("cast spell nr.? ");
    fgets(drbuf, 10, stdin);
    sp= atoi(drbuf);
    guy->encSpell= sp;
}

byte xposForMonster(byte numMonsters, byte mPos, byte mWidth) {
    byte width;
    width= 40 / numMonsters;
    return (width * mPos) + (width / 2) - (mWidth / 2);
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

void doMonsterTurn(monster *theMonster) {

    character *opponent;
    hitResult hitRes;
    byte row, column;

    row= theMonster->row;
    column= theMonster->column;

    if (theMonster->status == dead || theMonster->status == surrendered) {
        return;
    }

    if (theMonster->status == asleep) {
        // todo: handle wakeup timer
        return;
    }

    opponent= chooseRandomCharacter();
    doMonsterHit(theMonster, opponent, &hitRes);
    theMonster->hasDoneTurn= true;
    plotMonster(theMonster);

    clearText();
    cputs(theMonster->def->name);
    cputs(" attacks ");
    cputs(opponent->name);
    /*
    printf("%s (i %d at %d,%d) attacks %s:\n"
           "hit roll %d+%d (AC %d) vs. AC %d: ",
           theMonster->def->name, theMonster->initiative, row, column,
           opponent->name, hitRes.hitRoll, hitRes.hitBonus, hitRes.acHit,
           hitRes.toHit);*/
    if (hitRes.success) {
        cputs(": ");
        textcolor(BCOLOR_RED | CATTR_LUMA4);
        printf("\n%s takes %d points of damage.\n", opponent->name,
               hitRes.damage);
        opponent->aHP-= hitRes.damage;
        if (opponent->aHP <
            (-3 - bonusValueForAttribute(opponent->attributes[aCON]))) {
            textcolor(BCOLOR_RED | CATTR_LUMA5 | CATTR_BLINK);
            cprintf("%s dies!", opponent->name);
            opponent->status= dead;
            redrawParty();
        } else if (opponent->aHP <= 0) {
            cprintf("%s goes down!", opponent->name);
            opponent->status= down;
            opponent->aHP= 0; // cap HP at 0 regardless of damage
            redrawParty();
        }
    } else {
        cputs(" and misses.");
    }
    textcolor(BCOLOR_WHITE | CATTR_LUMA7);
    cgetc();
}

monster *opponentOnRank(byte rank) {

    byte i;
    byte found;
    byte liveMonsterCount;
    static monster *liveMonsters[10];
    monster *returnMonster;

    liveMonsterCount= 0;
    found= false;
    for (i= 0; i < gMonsterCount; i++) {
        returnMonster= gMonsterRoster[i];
        if (returnMonster->row == rank && returnMonster->hp > 0) {
            found= true;
            liveMonsters[liveMonsterCount++]= returnMonster;
        }
    }

    if (!found) {
        return NULL;
    }

    return liveMonsters[drand(liveMonsterCount)];
}

void doCharacterDamage(hitResult *result) {

    item *weapon;
    character *aCharacter;
    int hitDice;
    int bonus;

    aCharacter= result->theCharacter;

    weapon= getWeapon(aCharacter);
    hitDice= getHitDiceForCharacter(aCharacter);
    bonus= bonusValueForAttribute(aCharacter->attributes[aSTR]);

    if (weapon) {
        bonus+= (weapon->val3);
    }

    result->damageBonus= bonus;

    if (result->critical) {
        result->damage= hitDice + bonus;
    } else {
        result->damage= (drand(hitDice) + bonus);
    }
}

void doCharacterHit(character *aCharacter, byte rank, hitResult *result) {
    monster *opponent;

    result->theCharacter= aCharacter;

    opponent= opponentOnRank(rank);

    if (!opponent) {
        result->theMonster= NULL;
        return;
    }

    result->theMonster= opponent;

    result->toHit= opponent->def->armorClass;
    result->hitRoll= drand(20);
    result->critical= (result->hitRoll == 20);
    result->hitBonus= bonusValueForAttribute(
        aCharacter->attributes[aSTR]); // todo: DEX when using bow
    result->acHit= 20 - (result->hitRoll + result->hitBonus);
    result->success= ((result->acHit <= result->toHit) || result->critical);
    if (result->success) {
        doCharacterDamage(result);
    }
}

void performCharacterHitResult(hitResult *res) {

    cputs(res->theCharacter->name);
    if (!res->theMonster) {
        cputs(" stumps his feet\r\ninto the ground.\r\n");
        return;
    }

    if (res->theCharacter->currentEncounterCommand == ec_fireBow) {
        printf(" fires at rank %d ",res->theCharacter->encDestRank+1);
    } else {
        cputs(" attacks ");
        cputs(res->theMonster->def->name);
    }

    /*
    printf(":\nhit roll %d+%d (AC %d) vs. AC %d: ", res->hitRoll, res->hitBonus,
           res->acHit, res->toHit);
           */

    if (!res->success) {
        cputs(" and misses.");
        return;
    }

    cputc(':');
    textcolor(BCOLOR_BLUE | CATTR_LUMA5);
    printf("\n%s takes %d points of damage\n", res->theMonster->def->name,
           res->damage);

    res->theMonster->hp-= res->damage;
    
    if (res->theMonster->hp <= 0) {
        printf("%s dies!\n", res->theMonster->def->name);
        res->theMonster->status= dead;
        eraseMonster(res->theMonster);
    }
    textcolor(BCOLOR_WHITE | CATTR_LUMA7);
}

void doPartyTurn(byte idx) {
    character *theCharacter;
    encCommand encounterCommand;
    byte encSpell;
    byte encDestinationRank;
    hitResult hitRes;

    theCharacter= party[idx];
    encounterCommand= theCharacter->currentEncounterCommand;
    encSpell= theCharacter->encSpell;
    encDestinationRank= theCharacter->encDestRank;

    clearText();
    /*
    printf("should do %s (i %d): encC %d encS %d encDR %d\n",
           theCharacter->name, theCharacter->initiative, encounterCommand,
           encSpell, encDestinationRank);
           */

    switch (encounterCommand) {

    case ec_attack:
    case ec_fireBow:
        doCharacterHit(theCharacter, encDestinationRank, &hitRes);
        performCharacterHitResult(&hitRes);
        break;

    case ec_parry:
        printf("%s parries.\n", theCharacter->name);
        break;

    default:
        break;
    }

    hasDoneTurn[idx]= true;
    plotCharacter(idx, true);

    if (theCharacter->status != down) {
        cputs("\r\n--key--");
        cgetc();
    }
}

/**
 * @brief plots sprite spriteID at x,y using avariant if specified
 */
void plotSprite(byte x, byte y, byte spriteID, byte variant, byte colour) {
    byte i, j;
    byte *screenPtr;
    byte *colourPtr;
    byte charIdx;
    screenPtr= SCREEN + (x - 1 + (y * 40));
    colourPtr= COLOR_RAM + (x - 1 + (y * 40));
    charIdx= idxTable[spriteID] - 1;
    if (variant) {
        charIdx+= 9;
    }
    for (i= 0; i < 3; ++i) {
        for (j= 0; j < 3; ++j) {
            *(++screenPtr)= ++charIdx;
            *(++colourPtr)= colour;
        }
        screenPtr+= 37; // move on to next row (40 - 3 already printed chars)
        colourPtr+= 37;
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

void eraseMonster(monster *aMonster) {
    byte x, y;

    x= xposForMonster(gNumMonstersForRow[aMonster->row], aMonster->column, 3);
    y= ((2 - aMonster->row) * 4);
    eraseSprite(x, y);
}

void plotMonster(monster *aMonster) {
    byte x, y;

    x= xposForMonster(gNumMonstersForRow[aMonster->row], aMonster->column, 3);
    y= ((2 - aMonster->row) * 4);

    plotSprite(x, y, aMonster->def->spriteID, aMonster->hasDoneTurn,
               BCOLOR_BLUEGREEN | CATTR_LUMA5);
}

void plotCharacter(byte idx, byte variant) {
    byte x, y;

    x= xposForMonster(partyMemberCount(), idx, 3);
    y= 13;

    if (party[idx]->status == down) {
        plotSprite(x, y, 0, true, BCOLOR_LIGHTBLUE | CATTR_LUMA3);
        return;
    }

    if (party[idx]->status == dead) {
        plotSprite(x, y, 0, false, BCOLOR_PURPLE | CATTR_LUMA3);

        return;
    }

    plotSprite(x, y, party[idx]->spriteID, variant,
               BCOLOR_YELLOW | CATTR_LUMA5);
}

char *filenameForSpriteID(int id) {
    sprintf(sfname, "spr%c%c", 'a' + id / 16, 'a' + id % 16);
    return sfname;
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

    // sprintf(sfname, "spr%02x", id);
    spritefile= fopen(filenameForSpriteID(id), "rb");
    cputc('.');
    addr= (byte *)0xf000 + (gCurrentSpriteCharacterIndex * 8);
    // printf("\n%s -> %d @ $%x", sfname, gCurrentSpriteCharacterIndex, addr);
    if (spritefile) {
        fread(addr, 144, 1, spritefile);
        fclose(spritefile);
    } else {
        // printf("\n!spritefile %s not found", sfname);
        spritefile= fopen(filenameForSpriteID(1), "rb");
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

// clang-format off
#pragma code-name(push, "OVERLAY3");
// clang-format on

/*

           ==================================================
                          encounter overlay
           ==================================================

*/

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

void rebuildMonsterPositions(void) {
    byte i, row, x;
    byte column;
    monster *aMonster;
    byte rowOccupied;

    for (i= 0; i < gMonsterCount; i++) {
        aMonster= gMonsterRoster[i];
        if (aMonster->status == dead || aMonster->status == surrendered) {
            aMonster->row= -1;
        }
    }

    for (x= 0; x < 1; ++x) {
        for (row= 0; row < MONSTER_ROWS; row++) {
            rowOccupied= false;
            for (i= 0; i < gMonsterCount; i++) {
                aMonster= gMonsterRoster[i];
                if (aMonster->row == row && aMonster->status != dead &&
                    aMonster->status != surrendered) {
                    rowOccupied= true;
                }
            }
            if (!rowOccupied && row < 2) {
                for (i= 0; i < gMonsterCount; i++) {
                    aMonster= gMonsterRoster[i];
                    if (aMonster->row > 0) {
                        aMonster->row--;
                    }
                }
            }
        }
    }

    for (row= 0; row < MONSTER_ROWS; row++) {
        column= 0;
        for (i= 0; i < gMonsterCount; i++) {
            aMonster= gMonsterRoster[i];
            if (aMonster->row == row && aMonster->status != dead &&
                aMonster->status != surrendered) {
                aMonster->column= column++;
            }
            gNumMonstersForRow[row]= column;
        }
    }
}

monster *getMonsterAtPos(char row, char column) {
    monster *aMonster;
    char i;
    for (i= 0; i < gMonsterCount; i++) {
        aMonster= gMonsterRoster[i];
        if (aMonster->row == row && aMonster->column == column) {
            return aMonster;
        }
    }
    return NULL;
}

// TODO
monster *getMonsterForRow(char row) {
    monster *aMonster;
    char i;
    for (i= 0; i < gMonsterCount; i++) {
        aMonster= gMonsterRoster[i];
        if (aMonster->row == row) {
            return aMonster;
        }
    }
    return NULL;
}

encResult preEncounter(void) {

    static byte i, j;
    static byte totalMonsterCount;
    static byte livePartyMembersCount;
    static monster *aMonster;
    static char choice;
    character *aCharacter;

    monsterAuthorityLevel= 0;

    rebuildMonsterPositions();

    // display ranks and compute authority level while we're at it
    totalMonsterCount= 0;
    for (i= 0; i < gMonsterCount; i++) {
        aMonster= gMonsterRoster[i];
        if (aMonster->status == awake) {
            aMonster->hasDoneTurn= false;
            ++totalMonsterCount;
            monsterAuthorityLevel+= aMonster->level;
            monsterAuthorityLevel+= aMonster->def->courageModifier;
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

    do {

        clrscr();

        textcolor(BCOLOR_RED | CATTR_LUMA3);
        chlinexy(0, 10, 40);
        chlinexy(0, 16, 40);
        textcolor(BCOLOR_WHITE | CATTR_LUMA6);
        showCurrentParty(false);
        gotoxy(0, 12);

        for (i= 0; i < MONSTER_ROWS; ++i) {
            j= gNumMonstersForRow[i];
            if (j) {
                aMonster= getMonsterForRow(i);
                printf("Rank %d: %d %s(s)\n", i + 1, gNumMonstersForRow[i],
                       aMonster->def->name);
            }
        }

#ifdef DEBUG
        gotoxy(30, 17);
        printf("(P%d/M%d)", partyAuthorityLevel, monsterAuthorityLevel);
#endif
        gotoxy(0, 18);
        puts("F)ight       A)ccept Surrender");
        puts("G)reet       B)eg for mercy");
        puts("R)un away\n");
        cputs(">");
        cursor(1);

        do {
            choice= cgetc();
        } while (strchr("fagbr123456", choice) == NULL);

        if (choice >= '1' && choice <= '6') {
            inspectCharacter(choice - '1');
        }

    } while (strchr("fagbr", choice) == NULL);

    cursor(0);

    switch (choice) {
    case 'f':
        return encFight; // just fight
        break;

    case 'a':
        return checkSurrender();
        break;

    case 'g':
        return checkGreet();

    case 'b':
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

    byte i;
    monster *aMonster;

    for (i= 0; i < gMonsterCount; i++) {
        aMonster= gMonsterRoster[i];
        loadSpriteIfNeeded(aMonster->def->spriteID);
        aMonster->status= awake;
        aMonster->initiative=
            (byte)(drand(19)); // todo: honor monster attributes
    }
}

void prepareCharacters(void) {
    byte i;
    for (i= 0; i < partyMemberCount(); ++i) {
        addInventoryItem(0x24, party[i]); // add magical short bow for testing

        party[i]->initiative=
            (drand(20) + bonusValueForAttribute(party[i]->attributes[3]));
        loadSpriteIfNeeded(party[i]->spriteID);
    }
}

void getChoicesForPartyMember(byte idx) {
    character *guy;
    byte melee, ranged;
    char choice;
    char repeat;
    guy= party[idx];

    do {
        clearText();
        melee= false;
        ranged= false;
        repeat= false;
        cputs(guy->name);
        cputs(":\r\nA)ttack  S)lash  T)hrust  P)arry\r\nF)ire  C)ast "
              "spell  "
              "O)ptions\r\n>");
        cursor(1);

        do {
            choice= cgetc();
        } while (strchr("astfpco", choice) == NULL);

        switch (choice) {

        case 'a':
            if (getWeapon(guy)->type == it_missile) {
                guy->currentEncounterCommand= ec_fireBow;
                ranged= true;
            } else {
                guy->currentEncounterCommand= ec_attack;
                melee= true;
            }
            break;

        case 's':
            melee= true;
            if (guy->aClass == ct_fighter && guy->level >= 4) {
                guy->currentEncounterCommand= ec_slash;
            } else {
                guy->currentEncounterCommand= ec_attack;
            }
            break;

        case 't':
            melee= true;
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

        if (melee) {
            if (getWeapon(guy)->type != it_weapon) {
                cputs("fire\r\nequip a melee weapon first!\r\n--key--");
                cg_getkey();
                repeat= true;
            }
        }

        if (repeat == false) {
            if ((guy->currentEncounterCommand == ec_fireBow) ||
                (guy->aClass == ct_thief &&
                 (guy->currentEncounterCommand == ec_attack ||
                  guy->currentEncounterCommand == ec_slash ||
                  guy->currentEncounterCommand == ec_thrust))) {
                cprintf("%s at rank ",
                        gEncounterAction_p[guy->currentEncounterCommand]);
                do {
                    guy->encDestRank= cgetc() - '1';
                } while (guy->encDestRank > 3);
            }
        }

    } while (repeat == true);

    cursor(0);
}

void redrawMonsters(void) {
    byte c;
    monster *aMonster;

    for (c= 0; c < gMonsterCount; c++) {
        aMonster= gMonsterRoster[c];
        if (aMonster->row == -1) {
            continue;
        }
        if (aMonster->status != dead && aMonster->status != surrendered) {
            plotMonster(aMonster);
        } else {
            eraseMonster(aMonster);
        }
    }
}

void redrawParty(void) {
    byte j;
    for (j= 0; j < PARTYSIZE; ++j) {
        if (party[j]) {
            plotCharacter(j, hasDoneTurn[j]);
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

        for (j= 0; j < 6; j++) {
            hasDoneTurn[j]= false;
        }

        redrawMonsters();
        redrawParty();

        // main encounter loop

        gotChoices= false;
        do {
            for (j= 0; j < PARTYSIZE; ++j) {
                party[j]->currentEncounterCommand= ec_nothing;
                party[j]->encSpell= 0;
                party[j]->encDestRank= 0;
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
                        printf(" at rank %d", party[j]->encDestRank + 1);
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

        for (c= 30; c != 0; c--) {
            // while (iterateMonsters(&aMonster, &i, &j)) {
            for (i= 0; i < gMonsterCount; i++) {
                aMonster= gMonsterRoster[i];
                if (!isPartyDefeated()) {
                    if (aMonster->initiative == c) {
                        doMonsterTurn(aMonster);
                    }
                } else {
                    // kill off all down members and exit battle
                    for (j= 0; j < PARTYSIZE; ++j) {
                        party[j]->status= dead;
                    }
                    return encDead;
                }
            }
            for (j= 0; j < PARTYSIZE; ++j) {
                if (party[j] && party[j]->initiative == c) {
                    if (areMonstersDefeated()) {
                        return encWon;
                    } else {
                        doPartyTurn(j);
                    }
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

    byte i;
    monster *aMonster;

    for (i= 0; i < gMonsterCount; i++) {
        aMonster= gMonsterRoster[i];
        newCoins= 0;
        if (aMonster->status == dead || aMonster->status == asleep) {
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

    bordercolor(BCOLOR_BLACK);
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
