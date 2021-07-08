

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
dbmInfo *guildDBM;

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

void loadCityImages() {
    sprintf(drbuf, "city%d.dbm", gCurrentCityIndex + 1);
    cityDBM= cg_loadDBM(drbuf, NULL, NULL);
    guildDBM= cg_loadDBM("guild1.dbm", NULL, NULL);
}

void enterCityMode(void) {
    cg_go16bit(0, 0);
    cg_clrscr();
    cg_gotoxy(4, 12);
    cg_printf("Welcome to %s", gCities[gCurrentCityIndex]);
    loadCityImages();
    initGuild();
    initArmory();
    if (gPartyExperience || gPartyGold) {
        distributeSpoils();
    }
    runCityMenu();
    leaveCityMode();
}

void doGuild(void) {
    char *guildMenu[]= {"New guild member",
                        "Purge guild member",
                        "List guild members",
                        "Add to party",
                        "Drop from party",
                        "Train",
                        "Research spells",
                        "Exit",
                        NULL};

    static unsigned char quitGuild;
    static byte menuChoice;

    quitGuild= 0;

    while (!quitGuild) {
        cg_clrscr();
        cg_borders(true);
        cg_textcolor(COLOR_GRAY2);
        cg_displayDBMInfo(guildDBM, 1, 1);
        sprintf(drbuf, "%s Adventurer's Guild", gCities[gCurrentCityIndex]);
        cg_revers(1);
        cg_center(0, 0, 40, drbuf);
        cg_center(0, 16, 40, "Current party:");
        cg_revers(0);
        showCurrentParty(1, 17, false);
        cg_textcolor(COLOR_ORANGE);
        menuChoice= runMenu(guildMenu, 18, 2, true, false);
        cg_getkey();
        cg_titlec(COLOR_GREEN, 1, drbuf);

        cg_textcolor(COLOR_PURPLE);
        menuChoice= runBottomMenuN(guildMenu);

        if (menuChoice >= 100) {
            inspectCharacter(menuChoice - 100);
        }

        switch (menuChoice) {
        case 0:
            newGuildMember(gCurrentCityIndex);
            break;

        case 1:
            purgeGuildMember();
            break;

        case 2:
            addToParty();
            break;

        case 3:
            dropFromParty();
            break;

        case 6:
            quitGuild= true;
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
        cg_displayDBMInfo(cityDBM, 1, 1);
        showCurrentParty(0, 2, true);
        cg_gotoxy(3, 16);
        cg_textcolor(COLOR_GRAY3);
        cg_revers(1);
        cg_center(0, 16, 18, gCities[gCurrentCityIndex]);
        cg_revers(0);
        cg_setwin(0, 0, 40, 25);
        cg_textcolor(COLOR_CYAN);
        menuChoice= runBottomMenuN(cityMenu);
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
    char *cname;
    static character *newC; // the "cr" sign to the finished string...

    static char top; // screen top margin

    const char margin= 14;
    const char delSpaces= 40 - margin;

    char *menuEntries[10];

    cg_titlec(COLOR_CYAN, 0, "New Guild Member");

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
        menuEntries[i]= gRaces[i];
    }
    menuEntries[i]= NULL;
    race= runMenu(menuEntries, margin, top, true, false);

    for (i= top - 1; i < NUM_RACES + top + 3;
         cg_clearxy(margin, ++i, delSpaces))
        ;
    cg_putsxy(margin, top, gRaces[race]);

    ++top;

    cg_putsxy(2, top, "     Class:");
    for (i= 0; i < NUM_CLASSES; i++) {
        menuEntries[i]= gClasses[i];
    }
    menuEntries[i]= NULL;
    class= runMenu(menuEntries, margin, top, true, false);
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

        menuEntries[0]= "keep";
        menuEntries[1]= "reroll";
        menuEntries[2]= "quit";
        menuEntries[3]= NULL;

        c= runBottomMenu(menuEntries);

    } while (c == 1);

    if (c == 2)
        return;

    top= top + i + 4;
    cg_clearxy(0, top, 40);
    cg_putsxy(18, top + 1, "---------------");
    cg_putsxy(2, top, "Character name: ");
    cname= cg_input(16);
    // fgets(cname, 17, stdin); // see above
    // cname[strlen(cname) - 1]= 0;

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
    free(cname);
}

// clang-format off
#pragma code-name(pop);
// clang-format on
