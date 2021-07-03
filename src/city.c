

#include <c64.h>
//#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "armory.h"
#include "character.h"
#include "congui.h"
#include "globals.h"
#include "guild.h"
#include "utils.h"

#include "dungeon.h"
#include "menu.h"

const char *invError= "INVERR (%d)";
dbmInfo *cityDBM;

// clang-format off
#pragma code-name(push, "OVERLAY2");
// clang-format on

void runCityMenu(void);

void leaveCityMode(void) {
    cityCoordsT coords;

    coords= gCityCoords[gCurrentCityIndex];
    free(guild);
    releaseArmory();
    gCurrentDungeonIndex= coords.mapNr | 128;
    gStartXPos= coords.x;
    gStartYPos= coords.y;

    prepareForGameMode(gm_outdoor);
}

void distributeSpoils(void) {
    byte i;
    byte sharePerMember[PARTYSIZE];
    byte totalShares= 0;
    unsigned int moneyShare;
    unsigned int xpShare;
    if (partyMemberCount() == 0) {
        return;
    }
    cg_clrscr();
    cg_borders(true);
    cg_revers(1);
    cg_putsxy(2, 3, "Distribute gold and experience\n\n");
    cg_revers(0);
    cg_cursor(1);
    for (i= 0; i < partyMemberCount(); ++i) {
        cg_gotoxy(5, 6 + i);
        cg_printf("Shares for %-10s: ", party[i]->name);
        do {
            sharePerMember[i]= cg_getkey() - '0';
        } while (sharePerMember[i] < 1 || sharePerMember[i] > 3);
        cg_putc('0' + sharePerMember[i]);
        totalShares+= sharePerMember[i];
    }
    moneyShare= gPartyGold / totalShares;
    xpShare= gPartyExperience / totalShares;
    cg_gotoxy(0, 14);
    cg_printf("Each share is %u xp and %u coins.", moneyShare, xpShare);
    for (i= 0; i < partyMemberCount(); ++i) {
        party[i]->gold+= sharePerMember[i] * moneyShare;
        party[i]->xp+= sharePerMember[i] * xpShare;
    }
    cg_putsxy(1, 18, "--key--");
    cg_getkey();
}

void loadCityImage() {
    sprintf(drbuf, "city%d.dbm", gCurrentCityIndex + 1);
    cityDBM= cg_loadDBM(drbuf, NULL);
}

void enterCityMode(void) {
    cg_go16bit(0, 0);
    cg_clrscr();
    cg_gotoxy(4, 12);
    loadCityImage();
    cg_printf("Welcome to %s", gCities[gCurrentCityIndex]);
    sleep(1);
    initGuild();
    initArmory();
    if (gPartyExperience || gPartyGold) {
        distributeSpoils();
    }
    runCityMenu();
    leaveCityMode();
}

void doGuild(void) {
    const char menu[]= "  L)ist guild members  T)raining\n"
                       "  N)ew guild member    S)pells\n"
                       "  P)urge guild member\n"
                       "  A)dd to party\n"
                       "  D)rop from party\n"
                       "  eX)it guild\n";

    static unsigned char cmd;
    static unsigned char quitGuild;

    quitGuild= 0;

    while (!quitGuild) {
        sprintf(drbuf, "%s Guild", gCities[gCurrentCityIndex]);
        cg_titlec(COLOR_BROWN, COLOR_GREEN, 1, drbuf);
        showCurrentParty(false);
        cg_gotoxy(0, 14);
        cg_puts(menu);
        cg_putsxy(2, 22, "Command:");
        cg_cursor(1);
        do {
            cmd= cg_getkey();
        } while (strchr("lnpadxts123456", cmd) == NULL);

        cg_cursor(0);

        if (cmd >= '1' && cmd <= '6') {
            inspectCharacter(cmd - '1');
        }

        switch (cmd) {

        case 'a':
            addToParty();
            break;

        case 'd':
            dropFromParty();
            break;

        case 'n':
            newGuildMember(gCurrentCityIndex);
            break;

        case 'l':
            listGuildMembers();
            break;

        case 'p':
            purgeGuildMember();
            break;

        case 'x':
            return;
            break;

        default:
            break;
        }
    }
}

void saveGame() {
    cg_clrscr();
    cg_borders(true);
    cg_puts("\nPlease wait\nSaving economy...");
    saveArmory();
    cg_puts("Saving guild...");
    saveGuild();
    cg_puts("Saving party...");
    saveParty();
    cg_puts("\n\n...done.\n\n --key--");
    cg_getkey();
}

void runCityMenu(void) {

    static unsigned char cmd;
    static byte menuChoice;
    static unsigned char quitCity;
    char *cityMenu[]= {"Guild",  "Armory", "Inn", "Bank",
                       "Mystic", "Leave",  NULL};

    quitCity= 0;

    while (!quitCity) {
        cg_clrscr();
        cg_borders(true);
        cg_addGraphicsRect(1, 1, 15, 15, cityDBM->baseAdr);
        showCurrentParty(true);
        cg_gotoxy(3, 16);
        cg_textcolor(COLOR_LIGHTBLUE);
        cg_revers(1);
        cg_printf("%s (%d)", gCities[gCurrentCityIndex], gCurrentCityIndex + 1);
        cg_revers(0);
        cg_setwin(0, 0, 40, 25);
        cg_textcolor(COLOR_CYAN);
        menuChoice= runMenu(cityMenu);
        cmd= 0;

        if (menuChoice >= 100) {
            inspectCharacter(menuChoice - 100);
        }

        switch (menuChoice) {

        case 0:
            doGuild();
            break;

        case 1:
            doArmory();
            break;

        case 5:
            cg_clrscr();
            cg_gotoxy(0, 23);
            cg_printf("Really leave %s (y/n)?", gCities[gCurrentCityIndex]);
            do {
                cg_cursor(1);
                cmd= cg_getkey();
                cg_cursor(0);
            } while (strchr("yn", cmd) == NULL);
            if (cmd == 'y') {
                quitCity= 1;
            }
            cmd= 0;
            break;

        default:
            break;
        }
    }
}

void newGuildMember(byte city) {
    static byte i, c; // loop and input temp vars
    static byte race;
    static byte class;
    static attrT tempAttr[6];
    static attrT current;
    static signed char slot;
    static int tempHP;
    static int tempMP;
    static char cname[17];  // one more, because cc65 somehow adds
    static character *newC; // the "cr" sign to the finished string...

    static char top; // screen top margin

    const char margin= 14;
    const char delSpaces= 40 - margin;

    cg_titlec(COLOR_GREEN, COLOR_CYAN, 0, "New Guild Member");

    slot= nextFreeGuildSlot();
    if (slot == -1) {
        cg_textcolor(2);
        cg_puts("\nSorry, the guild is full."
                "\nPlease purge some inactive members"
                "before creating new ones.\n\n--key--");
        cg_getkey();
        return;
    }

    top= 5;
    newC= &guild[slot];

    cg_putsxy(2, top, "      Race:");
    for (i= 0; i < NUM_RACES; i++) {
        cg_gotoxy(margin, top + i);
        cg_printf("%d - %s", i + 1, gRaces[i]);
    }
    cg_putsxy(margin, top + 1 + i, "Your choice: ");
    do {
        race= cg_getkey() - '1';
    } while (race >= NUM_RACES);
    for (i= top - 1; i < NUM_RACES + top + 3;
         cg_clearxy(margin, ++i, delSpaces))
        ;
    cg_putsxy(margin, top, gRaces[race]);

    ++top;

    cg_putsxy(2, top, "     Class:");
    for (i= 0; i < NUM_CLASSES; i++) {
        cg_gotoxy(margin, top + i);
        cg_printf("%d - %s", i + 1, gClasses[i]);
    }
    cg_putsxy(margin, top + 1 + i, "Your choice:");
    do {
        class= cg_getkey() - '1';
    } while (class >= NUM_CLASSES);
    for (i= top - 1; i < NUM_CLASSES + top + 3;
         cg_clearxy(margin, ++i, delSpaces))
        ;
    cg_putsxy(margin, top, gClasses[class]);

    top+= 2;

    cg_putsxy(2, top, "Attributes:");
    do {
        for (i= 0; i < 6; i++) {
            current= 7 + (drand(12) + gRaceModifiers[race][i]);
            tempAttr[i]= current;
            cg_putsxy(margin, top + i, gAttributes[i]);
            cg_gotoxy(margin + 13, top + i);
            cg_printf("%2d %s", current, bonusStrForAttribute(current));
        }
        tempHP= 3 + drand(8) + bonusValueForAttribute(tempAttr[0]);
        tempMP= 3 + drand(8) + bonusValueForAttribute(tempAttr[1]);

        cg_gotoxy(margin, top + i + 1);
        cg_printf("Hit points   %2d", tempHP);

        cg_gotoxy(margin, top + i + 2);
        cg_printf("Magic points %2d", tempMP);

        cg_putsxy(margin, top + i + 4, "k)eep, r)eroll or q)uit? ");
        do {
            c= cg_getkey();
        } while (!strchr("rkq", c));
    } while (c == 'r');

    if (c == 'q')
        return;

    top= top + i + 4;
    cg_clearxy(0, top, 40);
    cg_putsxy(18, top + 1, "---------------");
    cg_putsxy(2, top, "Character name: ");
    fgets(cname, 17, stdin); // see above
    cname[strlen(cname) - 1]= 0;

    // copy temp char to guild
    newC->city= city;
    newC->guildSlot= slot;
    newC->status= awake;
    newC->aRace= race;
    newC->aClass= class;

    for (i= 0; i < NUM_ATTRS; i++) {
        newC->attributes[i]= tempAttr[i];
    }

    // empty inventory & known spells
    for (i= 0; i < INV_SIZE; i++) {
        newC->inventory[i]= 0;
    }
    for (i= 0; i < 8; i++) {
        newC->spellMap[i]= 0;
    }

    addInventoryItem(0xff, newC); // add white orb for testing
    newC->weapon= 0x01;           // add club
    newC->armor= 0x80;            // add robes
    newC->aMaxHP= tempHP;
    newC->aHP= tempHP;
    newC->aMaxMP= tempMP;
    newC->aMP= tempMP;
    newC->level= 1;
    newC->spriteID= 0x80 + newC->aRace;
    strcpy(newC->name, cname);
}

// clang-format off
#pragma code-name(pop);
// clang-format on
