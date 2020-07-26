
#include <cbm.h>
#include <conio.h>
#include <c64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "character.h"
#include "congui.h"
#include "guild.h"
#include "utils.h"

#include "cityUI.h"

character *guild;

static FILE *outfile;

void _listGuildMembers(void);
void listGuildMembers(void);

void flagError(char *e);

// clang-format off
#pragma code-name(push, "OVERLAY2");
// clang-format on


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
    cg_line(top+i+4,margin,gSecondaryAreaLeftX-1,32,0);
    cputsxy(margin, top+i+4, "Name: ");
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


void _listGuildMembers(void) {
    static byte i, x, y;
    static byte charsPerRow= GUILDSIZE / 2;
    character *tempChar;

    for (i= 0; i < GUILDSIZE; ++i) {
        if (guild[i].status != deleted) {
            tempChar= &guild[i];
            x= (20 * (i / charsPerRow));
            y= (4 + (i % charsPerRow));
            gotoxy(x, y);
            if (isInParty(i)) {
                cputc('*');
            } else {
                cputc(' ');
            }
            cprintf("%2d %.10s", i + 1, tempChar->name);
            gotoxy(x + 14, y);
            cprintf("%s-%d", gClassesS[tempChar->aClass], tempChar->city + 1);
        }
    }
}

void listGuildMembers(void) {
    cg_titlec(COLOR_LIGHTBLUE, COLOR_GREEN, 0,
              "Guild Members");
    _listGuildMembers();
    cputsxy(0, 23, "-- key --");
    cgetc();
}

void flagError(char *e) {
    textcolor(2);
    cursor(0);
    cclearxy(0, 22, 40);
    cputsxy(2, 22, e);
    textcolor(8);
    cputsxy(2, 23, "-- key --");
    cgetc();
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

    clearMenuArea();
    revers(1);
    textcolor(COLOR_GRAY2);
    cputsxy(0,partyMemberCount(),"(none)");
    textcolor(COLOR_CYAN);
    cg_center(gSecondaryAreaLeftX,gMenuAreaTopY+1,gSecondaryAreaWidth,"drop whom? ");

    pm = cg_verticalChooser(0,0,1,14,partyMemberCount()+1);
    if (pm==partyMemberCount()) {
        return;
    }

    free(party[pm]);
    party[pm]= NULL;
    cleanupParty();
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

void addToParty(void) {
    static char inbuf[3];
    signed char slot;
    unsigned char gmIndex;

    character *newPartyCharacter;

    cclearxy(0, 22, 40);
    slot= nextFreePartySlot();
    if (slot == -1) {
        flagError("no room in party");
        return;
    }
    cg_clear();
    cg_titlec(COLOR_BROWN, COLOR_YELLOW, 0, "Add guild member");

    _listGuildMembers();
    cputsxy(2, 22, "Add which guild member (0=cancel)?");
    cursor(1);
    fgets(inbuf, 3, stdin);
    gmIndex= atoi(inbuf);
    if (gmIndex == 0) {
        return;
    }
    --gmIndex;
    if (gmIndex >= GUILDSIZE) {
        flagError("What is it with you?!");
        return;
    }
    if (guild[gmIndex].status == deleted) {
        flagError("nobody there");
        return;
    }
    if (isInParty(gmIndex)) {
        flagError("already in party");
        return;
    }

    newPartyCharacter= malloc(sizeof(character));
    memcpy(newPartyCharacter, (void *)&guild[gmIndex], sizeof(character));
    party[slot]= newPartyCharacter;
}

void purgeGuildMember(void) {
    static char cnum[5];
    static byte idx;
    cg_titlec(COLOR_ORANGE, 2, 0, "Purge guild member");
    textcolor(COLOR_RED);
    _listGuildMembers();
    cputsxy(0, 22, "Purge which member (0=cancel)? ");
    fgets(cnum, 16, stdin);
    idx= atoi(cnum);
    if (idx == 0) {
        return;
    }
    idx--;
    if (idx >= GUILDSIZE) {
        flagError("Are you working in QA?");
        return;
    }
    if (isInParty(idx)) {
        flagError("Member is currently in the party!");
        return;
    }
    guild[idx].status= deleted;
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
