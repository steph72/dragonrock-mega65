

#include <c64.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "armory.h"
#include "character.h"
#include "congui.h"
#include "globals.h"
#include "guild.h"
#include "sprites.h"
#include "utils.h"

#include "dungeon.h"

const byte gSecondaryAreaRightX= 29;
const byte gSecondaryAreaWidth= 11;
const byte gMainAreaWidth= 29;
const byte gMenuAreaTopY= 7;
const byte gMainAreaTopY= 7;
const byte gStatusAreaTopY= 16;

const char *keyMsg= " -- key -- ";

const char *invError= "INVERR (%d)";

// clang-format off
#pragma code-name(push, "OVERLAY2");
// clang-format on

void runCityMenu(void);

void clearMenuArea(void) {
    cg_block(gSecondaryAreaRightX, gMenuAreaTopY, 39, gStatusAreaTopY - 1, 160,
             COLOR_GRAY1);
}

void clearStatusArea(void) {
    cg_block(gSecondaryAreaRightX, gStatusAreaTopY, 39, 24, 160, COLOR_BROWN);
}

void setupCityScreen(void) {
    cg_clear();
    cg_block(0, 0, 39, gMenuAreaTopY - 1, 160, COLOR_GRAY2);
    clearStatusArea();
    clearMenuArea();
}

void leaveCityMode(void) {
    free(guild);
    releaseArmory();
}

void distributeSpoils(void) {
    byte i;
    byte y;
    byte sharePerMember[PARTYSIZE];
    byte totalShares= 0;

    char *sharesItems[]= {"0", "1", "2", "3", ""};

    unsigned int moneyShare;
    unsigned int xpShare;
    if (partyMemberCount() == 0) {
        return;
    }

    revers(1);
    textcolor(COLOR_BROWN);
    cg_center(gSecondaryAreaRightX, gStatusAreaTopY + 1, gSecondaryAreaWidth,
              "dividing");
    cg_center(gSecondaryAreaRightX, gStatusAreaTopY + 3, gSecondaryAreaWidth,
              "gold and");
    cg_center(gSecondaryAreaRightX, gStatusAreaTopY + 4, gSecondaryAreaWidth,
              "experience");
    cg_center(gSecondaryAreaRightX, gStatusAreaTopY + 5, gSecondaryAreaWidth,
              "points");
    textcolor(COLOR_YELLOW);
    revers(1);
    cg_center(gSecondaryAreaRightX, gMenuAreaTopY + 1, gSecondaryAreaWidth,
              "Shares?");
    for (i= 0; i < partyMemberCount(); ++i) {
        cg_colorLine(i, 0, 39, COLOR_YELLOW);
        textcolor(COLOR_YELLOW);
        revers(1);
        gotoxy(0, i);
        cputs(party[i]->name);
        do {
            gotoxy(gSecondaryAreaRightX + 2, gMenuAreaTopY + 3);
            textcolor(COLOR_GRAY2);
            sharePerMember[i]= cg_horizontalMenu(COLOR_YELLOW, 1, sharesItems);
        } while (sharePerMember[i] < 1 || sharePerMember[i] > 3);
        gotoxy(16, i);
        cputc('0' + sharePerMember[i]);
        cg_colorLine(i, 0, 39, COLOR_GRAY2);
        totalShares+= sharePerMember[i];
    }
    moneyShare= gPartyGold / totalShares;
    xpShare= gPartyExperience / totalShares;
    textcolor(COLOR_GREEN);
    cg_colorLine(gMenuAreaTopY - 1, 0, 39, COLOR_GREEN);
    gotoxy(0, 6);
    sprintf(drbuf, "Each share is %u xp and %u coins.", moneyShare, xpShare);
    cg_center(0, gMenuAreaTopY - 1, 40, drbuf);
    for (i= 0; i < partyMemberCount(); ++i) {
        party[i]->gold+= sharePerMember[i] * moneyShare;
        party[i]->xp+= sharePerMember[i] * xpShare;
    }
    gPartyExperience= 0;
    gPartyGold= 0;
    clearStatusArea();
    clearMenuArea();
    cg_getkeyP(gSecondaryAreaRightX + 1, gStatusAreaTopY + 2, "-- key --");
}

void displayCityTitle(void) {
    byte xstart;
    cg_clear();
    textcolor(COLOR_CYAN);
    sprintf(drbuf, " Welcome to %s ", gCities[gCurrentCityIndex]);
    xstart= 20 - ((strlen(drbuf) / 2));
    // bars above and below title
    cg_line(11, xstart, xstart + strlen(drbuf) - 1, 111, COLOR_CYAN);
    cg_line(13, xstart, xstart + strlen(drbuf) - 1, 119, COLOR_CYAN);
    revers(1);
    // title
    cg_center(0, 12, 40, drbuf);
}

void enterCityMode(void) {
    displayCityTitle();
    initGuild();
    initArmory();
    sleep(2);

    setupCityScreen();

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

    char *guildMenu[]= {"Spells", "Training", "Add",   "New",  "Drop",
                        "Rename", "Inspect",  "Purge", "Exit", ""};

    static unsigned char cmd;
    static unsigned char quitGuild;

    quitGuild= 0;

    while (!quitGuild) {
        setupCityScreen();
        revers(1);
        textcolor(COLOR_BROWN);
        cg_center(gSecondaryAreaRightX, gStatusAreaTopY + 2,
                  gSecondaryAreaWidth, gCities[gCurrentCityIndex]);
        cg_center(gSecondaryAreaRightX, gStatusAreaTopY + 3,
                  gSecondaryAreaWidth, "guild");
        textcolor(COLOR_GRAY2);
        showCurrentParty(false);
        gotoxy(gSecondaryAreaRightX, gMenuAreaTopY);
        cmd= cg_menu(gSecondaryAreaWidth, COLOR_GRAY1, guildMenu);

        switch (cmd) {

        case 3:
            newGuildMember(gCurrentCityIndex);
            break;

        case 8:
            quitGuild= 1;
            break;

        default:
            break;
        }
    }

    /*
        cgetc();

        sprintf(drbuf, "%s Guild", gCities[gCurrentCityIndex]);
        cg_titlec(COLOR_BROWN, COLOR_GREEN, 1, drbuf);
        showCurrentParty(false);
        gotoxy(0, 14);
        puts(menu);
        cputsxy(2, 22, "Command:");
        cursor(1);
        do {
            cmd= cgetc();
        } while (strchr("lnpadxts123456", cmd) == NULL);

        cursor(0);

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
        */
}

void leaveCity(void) {}

void showCitySprites(byte enabled) {
    byte i;
    if (enabled)
        initSprites();
    for (i= 0; i < 6; ++i) {
        setSpriteEnabled(i, enabled);
        if (enabled) {
            setSpriteXExpand(i, 0);
            setSpriteYExpand(i, 0);
            setSpriteColor(i, 2 + i);
            putSprite(i, 52 + (64 * (i / 2)), 130 + (64 * (i % 2)));
        }
    }
}

void runCityMenu(void) {

    char marker[]= {169, 127};
    char *cityKeys[]= {"Cast", "Use item", "Save game", ""};
    char *cityServices[]= {"Guild", "Armory", "Bank",
                           "Inn",   "Mystic", "Leave city"};

    static byte menuX, menuY;
    static byte cityItem;
    static unsigned char cmd;
    static unsigned char quitCity;

    quitCity= 0;

    while (!quitCity) {
        setupCityScreen();
        showCitySprites(1);
        revers(1);
        textcolor(COLOR_BROWN);
        cg_center(gSecondaryAreaRightX, gStatusAreaTopY + 2,
                  gSecondaryAreaWidth, gCities[gCurrentCityIndex]);
        sprintf(drbuf, "(%d)", gCurrentCityIndex + 1);
        cg_center(gSecondaryAreaRightX, gStatusAreaTopY + 3,
                  gSecondaryAreaWidth, drbuf);
        textcolor(COLOR_GRAY2);
        showCurrentParty(false);
        cg_verticalList(gSecondaryAreaRightX + 1, gMenuAreaTopY + 1, 1, 0,
                        COLOR_GRAY1, cityKeys);
        cg_block(gSecondaryAreaRightX + 1, gMenuAreaTopY + 1,
                 gSecondaryAreaRightX + 1, gMenuAreaTopY + 3, 0, COLOR_GRAY2);

        menuX= 0;
        menuY= 0;

        do {
            cityItem= menuX + (3 * menuY);
            textcolor(COLOR_GREEN);
            revers(1);
            cg_line(gMenuAreaTopY + 5, gSecondaryAreaRightX, 39, 160,
                    COLOR_GREEN);
            cputsxy(gSecondaryAreaRightX + 1, gMenuAreaTopY + 5,
                    cityServices[cityItem]);

            textcolor(COLOR_WHITE);
            cputsxy((4 + (8 * menuX)), 13 + (8 * menuY), marker);

            while (!kbhit()) {
                cg_stepColor();
            }
            cmd= cgetc();
            revers(0);
            cputsxy((4 + (8 * menuX)), 13 + (8 * menuY), "  ");

            switch (cmd) {
            case 29: // cursor right
                if (menuX < 2)
                    menuX++;
                break;

            case 157: // cursor left
                if (menuX > 0)
                    menuX--;
                break;

            case 145: // cursor up
                if (menuY > 0)
                    menuY--;
                break;

            case 17: // cursor down
                if (menuY < 1)
                    menuY++;
                break;

            default:
                break;
            }

        } while (cmd != 13);

        showCitySprites(0);

        switch (cityItem) {

        case 0:
            doGuild();
            break;

        case 1:
            doArmory();
            break;

        case 5:
            quitCity= true;
            break;

        default:
            break;
        }
    }

    /*
            gotoxy(0, 14);
            puts(menu);
            cputsxy(8, 21, "Command:");
            cursor(1);

            do {
                cmd= cgetc();
            } while (strchr("agmiblcus123456", cmd) == NULL);

            cursor(0);

            if (cmd >= '1' && cmd <= '6') {
                inspectCharacter(cmd - '1');
            }

            switch (cmd) {

            case 'l':
                cg_clear();
                gotoxy(0, 23);
                printf("Really leave %s (y/n)?", gCities[gCurrentCityIndex]);
                do {
                    cursor(1);
                    cmd= cgetc();
                    cursor(0);
                } while (strchr("yn", cmd) == NULL);
                if (cmd == 'y') {
                    quitCity= 1;
                }
                cmd= 0;
                break;

            case 'a':
                doArmory();
                break;

            case 'g':
                doGuild();
                break;

            case 's':
                cg_clear();
                cg_borders();
                puts("\nPlease wait\nSaving economy...");
                saveArmory();
                puts("Saving guild...");
                saveGuild();
                puts("Saving party...");
                saveParty();
                puts("\n\n...done.\n\n --key--");
                cgetc();
                break;

            default:
                break;
            }
        }
        */
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

    const char margin= 3;
    const char delSpaces= 40 - margin;

    char *rollMenu[]= {"keep", "reroll", "quit", ""};

    char *racesMenu[NUM_RACES + 1];
    char *classesMenu[NUM_CLASSES + 1];

    textcolor(COLOR_RED);
    revers(1);
    cputsxy(3, gMainAreaTopY + 1, " = new guild member = ");

    slot= nextFreeGuildSlot();
    if (slot == -1) {
        textcolor(2);
        puts("\nSorry, the guild is full."
             "\nPlease purge some inactive members"
             "before creating new ones.\n\n--key--");
        cgetc();
        return;
    }

    newC= &guild[slot];

    for (i= 0; i < NUM_RACES; i++) {
        racesMenu[i]= gRaces[i];
    }
    racesMenu[i]= ""; // end marker for menu

    clearMenuArea();
    gotoxy(gSecondaryAreaRightX, gMenuAreaTopY + 1);
    race= cg_menu(gSecondaryAreaWidth, COLOR_GRAY1, racesMenu);
    textcolor(COLOR_LIGHTGREEN);
    cputsxy(3, gMainAreaTopY + 3, gRaces[race]);

    for (i= 0; i < NUM_CLASSES; i++) {
        classesMenu[i]= gClasses[i];
    }
    classesMenu[i]= "";

    clearMenuArea();
    gotoxy(gSecondaryAreaRightX, gMenuAreaTopY + 1);
    class= cg_menu(gSecondaryAreaWidth, COLOR_GRAY1, classesMenu);
    textcolor(COLOR_LIGHTGREEN);
    cputsxy(25 - strlen(gClasses[class]), gMainAreaTopY + 3, gClasses[class]);
    top= gMainAreaTopY + 5;
    cg_block(3, top, 24, top + 8, 160, COLOR_LIGHTGREEN);

    do {
        textcolor(COLOR_LIGHTGREEN);

        for (i= 0; i < 6; i++) {
            current= 7 + (drand(12) + gRaceModifiers[race][i]);
            tempAttr[i]= current;
            cputsxy(margin, top + i, gAttributes[i]);
            gotoxy(margin + 13, top + i);
            cprintf("%2d %s", current, bonusStrForAttribute(current));
        }
        tempHP= 3 + drand(8) + bonusValueForAttribute(tempAttr[0]);
        tempMP= 3 + drand(8) + bonusValueForAttribute(tempAttr[1]);

        gotoxy(margin, top + i + 1);
        cprintf("Hit points   %2d", tempHP);

        gotoxy(margin, top + i + 2);
        cprintf("Magic points %2d", tempMP);

        gotoxy(margin, top + i + 4);
        c= cg_horizontalMenu(COLOR_YELLOW, 1, rollMenu);

    } while (c == 1);

    if (c == 2)
        return;

    top= top + i + 4;
    cclearxy(0, top, 40);
    cputsxy(18, top + 1, "---------------");
    cputsxy(2, top, "Character name: ");
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
