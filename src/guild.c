
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

#include "congui.h"
#include "types.h"
#include "config.h"
#include "debug.h"

character *guild;
character *party;

void initGuildMem(void);
signed char nextFreeGuildSlot(void);
void newGuildMember(void);
void listGuildMembers(void);

void listGuildMembers(void)
{
    byte i;
    char num[3];
    cg_titlec(8, 5, 0, "Guild Members");

    for (i = 0; i < GUILDSIZE; ++i)
    {
        itoa(i, num, 10);
        if (guild[i].status != deleted)
        {
            cputsxy(2 + 18 * (i / 20), 4 + (i % 20), num);
            cputsxy(5 + 18 * (i / 20), 4 + (i % 20), guild[i].name);
        }
    }
    cgetc();
}

void newGuildMember(void)
{
    byte i, c; // loop and input temp vars
    byte race;
    byte class;
    attrT tempAttr[6];
    signed char slot;
    int tempHP;
    int tempMP;
    char attrVal[4]; // temp var for displaying attribute values
    char cname[17];
    character *newC;

    char top = 5; // screen top margin

    const char margin = 14;
    const char delSpaces = 40 - margin;

    cg_titlec(8, 5, 0, "New Guild Member");

    slot = nextFreeGuildSlot();
    if (slot == -1)
    {
        textcolor(2);
        puts("\nSorry, the guild is full.");
        puts("\nPlease purge some inactive members");
        puts("before creating new ones.\n\n--key--");
        cgetc();
        return;
    }

    newC = &guild[slot];

    cputsxy(2, top, "      Race:");
    for (i = 0; i < NUM_RACES; i++)
    {
        cputcxy(margin, top + i, '1' + i);
        cputsxy(margin + 1, top + i, " - ");
        cputs(gRaces[i]);
    }
    cputsxy(margin, top + 1 + i, "Your choice: ");
    do
    {
        race = cgetc() - '1';
    } while (race >= NUM_RACES);
    for (i = top - 1; i < NUM_RACES + top + 3; cclearxy(margin, ++i, delSpaces))
        ;
    cputsxy(margin, top, gRaces[race]);

    ++top;

    cputsxy(2, top, "     Class:");
    for (i = 0; i < NUM_CLASSES; i++)
    {
        cputcxy(margin, top + i, '1' + i);
        cputsxy(margin + 1, top + i, " - ");
        cputs(gClasses[i]);
    }
    cputsxy(margin, top + 1 + i, "Your choice:");
    do
    {
        class = cgetc() - '1';
    } while (class >= NUM_CLASSES);
    for (i = top - 1; i < NUM_CLASSES + top + 3; cclearxy(margin, ++i, delSpaces))
        ;
    cputsxy(margin, top, gClasses[class]);

    top += 2;

    cputsxy(2, top, "Attributes:");
    do
    {
        for (i = 0; i < 6; i++)
        {
            tempAttr[i] = 3 + (rand() % 16) + gRaceModifiers[race][i];
            itoa(tempAttr[i], attrVal, 10);
            cputsxy(margin, top + i, gAttributes[i]);
            cputsxy(margin + 13, top + i, attrVal);
            cputs("  ");
        }
        tempHP = 3 + (rand() % 8);
        tempMP = 3 + (rand() % 8);
        gotoxy(margin, top + i + 1);
        cprintf("Hit points  %d", tempHP);

        /* cputsxy(margin, top + i + 1, "Hit points   ");
        itoa(tempHP, attrVal, 10);
        cputs(attrVal);
        cputs("  ");
        */
        gotoxy(margin, top + i + 2);
        cprintf("Magic points %d", tempMP);
        /*
        cputsxy(margin, top + i + 2, "Magic points ");
        itoa(tempMP, attrVal, 10);
        cputs(attrVal);
        cputs("  ");
        */
        cputsxy(margin, top + i + 4, "Re-roll? (y/n) ");
        c = cgetc();
    } while (c != 'n');
    top = top + i + 4;
    cclearxy(0, top, 40);
    cputsxy(2, top, "Character name: ");
    fgets(cname, 16, stdin);

    // copy temp char to guild
    newC->status = alive;
    newC->aRace = race;
    newC->aClass = class;
    newC->aSTR = tempAttr[0];
    newC->aINT = tempAttr[1];
    newC->aWIS = tempAttr[2];
    newC->aDEX = tempAttr[3];
    newC->aCON = tempAttr[4];
    newC->aCHR = tempAttr[5];
    newC->aMaxHP = tempHP;
    newC->aHP = tempHP;
    newC->aMaxMP = tempMP;
    newC->aMP = tempMP;
    strcpy(newC->name, cname);
}

signed char nextFreeGuildSlot(void)
{
    unsigned char idx = -1;
    while (++idx < GUILDSIZE)
    {
        if (guild[idx].status == deleted)
        {
            return idx;
        }
    }
    return -1;
}

void initGuild(void)
{
    initGuildMem();
}

void initGuildMem(void)
{
    unsigned int sizeBytes = 0;
    sizeBytes = GUILDSIZE * sizeof(character);
    guild = (character *)malloc(sizeBytes);
    if (guild == NULL)
    {
        puts("???fatal: no memory for guild");
        exit(0);
    }
    bzero(guild, sizeBytes);

    sizeBytes = PARTYSIZE * sizeof(character);
    party = (character *)malloc(sizeBytes);
    if (party == NULL)
    {
        puts("???fatal: no memory for party");
        exit(0);
    }
    bzero(party, sizeBytes);
}