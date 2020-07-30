
#include <c64.h>
#include <cbm.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"
#include "congui.h"
#include "globals.h"
#include "guild.h"
#include "utils.h"

#include "cityUI.h"

character *guild;

static FILE *outfile;

void _listGuildMembers(void);
void listGuildMembers(void);


// clang-format off
#pragma code-name(push, "OVERLAY2");
// clang-format on

void setupGuildScreen() {
    setupCityScreen();
    revers(1);
    textcolor(COLOR_BROWN);
    cg_center(gSecondaryAreaLeftX, gStatusAreaTopY + 2, gSecondaryAreaWidth,
              gCities[gCurrentCityIndex]);
    cg_center(gSecondaryAreaLeftX, gStatusAreaTopY + 3, gSecondaryAreaWidth,
              "guild");
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
    cputsxy(3, gMainAreaTopY + 1, " * New Guild Member * ");

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
    gotoxy(gSecondaryAreaLeftX, gMenuAreaTopY + 1);
    race= cg_menu(gSecondaryAreaWidth, COLOR_GRAY1, racesMenu);
    textcolor(COLOR_LIGHTGREEN);
    cputsxy(3, gMainAreaTopY + 3, gRaces[race]);

    for (i= 0; i < NUM_CLASSES; i++) {
        classesMenu[i]= gClasses[i];
    }
    classesMenu[i]= "";

    clearMenuArea();
    gotoxy(gSecondaryAreaLeftX, gMenuAreaTopY + 1);
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
        clearMenuArea();
        c= cg_horizontalMenu(COLOR_YELLOW, 1, rollMenu);

    } while (c == 1);

    if (c == 2)
        return;

    textcolor(COLOR_LIGHTBLUE);
    cg_line(top + i + 4, margin, gSecondaryAreaLeftX - 1, 32, 0);
    cputsxy(margin, top + i + 4, "Name: ");
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

void cleanupParty(void) {
    byte i;
    for (i= 0; i < PARTYSIZE - 1; ++i) {
        if (party[i] == NULL) {
            if (party[i + 1] != NULL) {
                party[i]= party[i + 1];
                party[i + 1]= NULL;
            }
        }
    }
}

void dropFromParty(void) {
    static byte pm;

    do {
        clearPartyArea();
        textcolor(COLOR_GRAY2);
        revers(1);
        showCurrentParty(false,false);
        clearMenuArea();
        cputsxy(0, partyMemberCount(), "  (exit)");
        textcolor(COLOR_CYAN);
        cg_center(gSecondaryAreaLeftX, gMenuAreaTopY + 1, gSecondaryAreaWidth,
                  "drop whom? ");

        pm= cg_verticalChooser(0, 0, 1, 14, partyMemberCount() + 1,0);

        if (pm == partyMemberCount()) { // "exit" item?
            return;
        }

        free(party[pm]);
        party[pm]= NULL;
        cleanupParty();
    } while (1);
}

byte isInParty(byte guildIdx) {
    static byte i;
    for (i= 0; i < PARTYSIZE; i++) {
        if (party[i] && party[i]->guildSlot == guildIdx) {
            return true;
        }
    }
    return false;
}

byte chooseGuildMember(byte initialChoice) {

    signed char offset, guildIndex;
    byte y, row, choice;
    byte cmd;

    offset= -5;
    choice= initialChoice;

    textcolor(COLOR_ORANGE);
    cputsxy(0, gMainAreaTopY, "Name        Stat  Class  Twn#");
    textcolor(COLOR_GRAY1);

    do {
        for (row= 0; row < 17; ++row) {
            y= gMainAreaTopY + 1 + row;
            guildIndex= row + offset + choice - 1;
            cg_line(gMainAreaTopY + 1 + row, 0, gMainAreaRightX, 160,
                    COLOR_GRAY1);
            if (guildIndex == -1) {
                cputsxy(0, y, "(exit)");
            }
            if (guildIndex >= 0 && guildIndex < GUILDSIZE) {
                if (guild[guildIndex].status != deleted) {
                    cputsxy(0, y, guild[guildIndex].name);
                    if (isInParty(guildIndex)) {
                        cputsxy(12, y, "in party");
                    } else {
                        cputsxy(12, y, gStateDesc[guild[guildIndex].status]);
                        cputsxy(18, y, gClasses[guild[guildIndex].aClass]);
                        cputcxy(26, y, '1' + guild[guildIndex].city);
                    }
                } else {
                    cputsxy(0, y, "-empty-");
                }
            }
        }
        cg_line(gMainAreaTopY + 1 + 5, 0, gMainAreaRightX, 0, 1);

        while (!kbhit()) {
            cg_stepColor();
        }
        cmd= cgetc();

        switch (cmd) {
        case 17: // down
            if (choice < GUILDSIZE)
                choice++;

            break;
        case 145: // up
            if (choice > 0)
                choice--;

        default:
            break;
        }
    } while (cmd != 13);

    cg_line(gMainAreaTopY + 1 + 5, 0, gMainAreaRightX, 0, COLOR_GRAY1);

    return choice;
}

void addToParty(void) {

    signed char slot;
    unsigned char gmIndex;

    character *newPartyCharacter;

    gmIndex= 0;

    do {

        setupGuildScreen();
        textcolor(COLOR_CYAN);
        cg_block(gSecondaryAreaLeftX, gMenuAreaTopY, 39, gStatusAreaTopY - 1,
                 160, COLOR_GRAY2);
        cg_center(gSecondaryAreaLeftX, gMenuAreaTopY + 1, gSecondaryAreaWidth,
                  "add whom?");

        textcolor(COLOR_GRAY2);
        revers(1);
        showCurrentParty(false,false);

        textcolor(COLOR_GRAY1);
        gmIndex= chooseGuildMember(gmIndex + 1);

        if (gmIndex == 0) {
            return;
        }
        --gmIndex;

        if (guild[gmIndex].status == deleted) {
            continue;
        }
        if (isInParty(gmIndex)) {
            flagError("already in party");
            continue;
        }

        slot= nextFreePartySlot();
        if (slot == -1) {
            flagError("no room in party");
            return;
        }

        newPartyCharacter= malloc(sizeof(character));
        memcpy(newPartyCharacter, (void *)&guild[gmIndex], sizeof(character));
        party[slot]= newPartyCharacter;

    } while (1);
}

void purgeGuildMember(void) {

    signed char slot;
    unsigned char gmIndex;

    character *newPartyCharacter;

    gmIndex= 0;
    slot= 0;

    setupGuildScreen();
    textcolor(COLOR_CYAN);
    cg_block(gSecondaryAreaLeftX, gMenuAreaTopY, 39, gStatusAreaTopY - 1, 160,
             COLOR_GRAY2);
    cg_center(gSecondaryAreaLeftX, gMenuAreaTopY + 1, gSecondaryAreaWidth,
              "purge whom?");

    do {

        slot= chooseGuildMember(slot + 1);

        if (slot == 0)
            return;

        slot--;

        if (isInParty(slot)) {
            flagError("Member is currently in the party!");
            return;
        }

        guild[slot].status= deleted;
    } while (1);
}

signed char nextFreePartySlot(void) {
    signed char idx= -1;
    while (++idx < PARTYSIZE) {
        if (party[idx] == NULL) {
            return idx;
        }
    }
    return -1;
}

signed char nextFreeGuildSlot(void) {
    signed char idx= -1;
    while (++idx < GUILDSIZE) {
        if (guild[idx].status == deleted) {
            return idx;
        }
    }
    return -1;
}

void saveGuild(void) {
    outfile= fopen("gdata", "w");
    fwrite(guild, GUILDSIZE * sizeof(character), 1, outfile);
    fclose(outfile);
}

void saveParty(void) {
    static byte i, c;
    outfile= fopen("pdata", "w");
    c= partyMemberCount();
    fputc(c, outfile);
    for (i= 0; i < c; ++i) {
        fwrite(party[i], sizeof(character), 1, outfile);
    }
    fclose(outfile);
}

byte loadGuild(void) {
    static FILE *infile;

    infile= fopen("gdata", "r");
    if (!infile) {
        return false;
    }
    fread(guild, GUILDSIZE * sizeof(character), 1, infile);
    fclose(infile);

    return true;
}

byte initGuild() {
    initGuildMem();
    return loadGuild();
}

void initGuildMem(void) {
    static unsigned int sizeBytes= 0;
    sizeBytes= GUILDSIZE * sizeof(character);
    guild= (character *)malloc(sizeBytes);
    if (guild == NULL) {
        puts("???fatal: no memory for guild");
        exit(0);
    }
    bzero(guild, sizeBytes);
}

// clang-format off
#pragma code-name(pop);
// clang-format on
