#include "encounter.h"
#include <unistd.h>

byte gCurrentSpriteCharacterIndex;
byte idxTable[255]; // sprite index cache
static char sfname[8];

unsigned int partyAuthorityLevel;
unsigned int monsterAuthorityLevel;

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
    for (i= 0; i < 3; ++i) {
        for (j= 0; j < 3; ++j) {
            *(++screenPtr)= ++charIdx;
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

    plotSprite(x, y, party[idx]->spriteID);
}

void loadSprite(byte id) {
    byte *addr;
    FILE *spritefile;

    sprintf(sfname, "spr%03d", id);
    //spritefile= fopen(sfname, "rb");
    spritefile=fopen("spr128","rb");
    addr= (byte *)0xf000 + (gCurrentSpriteCharacterIndex * 8);
    printf("%s -> %d @ $%x", sfname,
           gCurrentSpriteCharacterIndex, addr);
    if (spritefile) {
        fread(addr,144,1,spritefile);
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

byte checkSurrender() { return 0; }

byte preEncounter(void) {
    static byte i, j;
    static byte count;
    static byte totalMonsterCount;
    static byte livePartyMembersCount;
    static monster *aMonster;
    static char *outName;
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
    for (i= 0; i < MONSTER_ROWS; ++i) {
        count= 0;
        for (j= 0; j < MONSTER_SLOTS; ++j) {
            if (gMonsterRow[i][j]) {
                ++count;
                ++totalMonsterCount;
                aMonster= gMonsterRow[i][j];
                monsterAuthorityLevel+= aMonster->level;
                monsterAuthorityLevel+= aMonster->def->courageModifier;
            }
        }
        if (count == 1) {
            outName= aMonster->def->name;
        } else {
            outName= pluralname(aMonster->def);
        }
        printf("Rank %d: %d %s\n", i + 1, count, outName);
    }
    monsterAuthorityLevel/= totalMonsterCount;

    // calc party authority level
    partyAuthorityLevel= 0;
    livePartyMembersCount= 0;
    for (i= 0; i < partyMemberCount(); ++i) {
        aCharacter= party[i];
        if (aCharacter->status == alive) {
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
    } while (choice < '1' || choice > '5');
    cursor(0);

    switch (choice) {
    case '1':
        return 0; // just fight
        break;

    case '2':
        return checkSurrender();
        break;

    default:
        break;
    }

    return 0;
}

encResult doEncounter(void) {

    byte c, i, j;
    monster *aMonster;

    bordercolor(BCOLOR_BLACK);
    bgcolor(BCOLOR_BLACK);
    textcolor(BCOLOR_WHITE | CATTR_LUMA6);

    clrscr();
    puts("An encounter!\n");
    sleep(1);
    cg_emptyBuffer();
    preEncounter();

    setSplitEnable(1);
    cg_clear();

    gotoxy(0, 17);

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