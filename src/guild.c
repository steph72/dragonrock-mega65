
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

#include "congui.h"
#include "types.h"
#include "config.h"
#include "guild.h"

character *guild;
character *party[PARTYSIZE];

void initGuildMem(void);
void loadGuild(void);
signed char nextFreeGuildSlot(void);
signed char nextFreePartySlot(void);

void newGuildMember(byte city);
void _listGuildMembers(void);
void listGuildMembers(void);

void flagError(char *e);

void _listGuildMembers(void)
{
    static byte i, x, y;
    static byte charsPerRow = GUILDSIZE / 2;
    character *tempChar;

    for (i = 0; i < GUILDSIZE; ++i)
    {
        if (guild[i].status != deleted)
        {
            tempChar = &guild[i];
            x = (20 * (i / charsPerRow));
            y = (4 + (i % charsPerRow));
            gotoxy(x, y);
            cprintf("%2d %.10s", i + 1, tempChar->name);
            gotoxy(x + 14, y);
            cprintf("%s-%d", gClassesS[tempChar->aClass], tempChar->city + 1);
        }
    }
}

void listGuildMembers(void)
{
    cg_titlec(8, 5, 0, "Guild Members");
    _listGuildMembers();
    cputsxy(0, 23, "-- key --");
    cgetc();
}

void flagError(char *e)
{
    textcolor(2);
    cursor(0);
    cclearxy(0,22,40);
    cputsxy(2,22,e);
    textcolor(8);
    cputsxy(2,23, "-- key --"); 
    cgetc();
}

void addToParty(void)
{
    static char inbuf[3];
    signed char slot;
    static byte i;
    unsigned char gmIndex;
    cclearxy(0, 22, 40);
    slot = nextFreePartySlot();
    if (slot == -1)
    {
        flagError("no room in party");
        return;
    }
    cputsxy(2, 22, "Add which guild member (0=cancel)?");
    cursor(1);
    fgets(inbuf, 3, stdin);
    gmIndex = atoi(inbuf);
    if (gmIndex == 0)
    {
        return;
    }
    --gmIndex;
    if (gmIndex >= GUILDSIZE)
    {
        flagError("you wish");
        return;
    }
    if (guild[gmIndex].status == deleted)
    {
        flagError("nobody there");
        return;
    }
    for (i = 0; i < PARTYSIZE; ++i)
    {
        if (party[i] == &guild[gmIndex])
        {
            flagError("already in party");
            return;
        }
    }

    party[slot] = &guild[gmIndex];
}

void purgeGuildMember(void)
{
    static char cnum[5];
    static byte idx;
    cg_titlec(8, 2, 0, "Purge guild member");
    textcolor(8);
    _listGuildMembers();
    textcolor(2);
    cputsxy(0, 22, "Purge which member (0=cancel)? ");
    fgets(cnum, 16, stdin);
    idx = atoi(cnum);
    if (idx == 0)
    {
        return;
    }
    idx--;
    guild[idx].status = deleted;
}

void newGuildMember(byte city)
{
    static byte i, c; // loop and input temp vars
    static byte race;
    static byte class;
    static attrT tempAttr[6];
    static attrT current;
    static signed char slot;
    static int tempHP;
    static int tempMP;
    static char cname[17];
    static character *newC;

    static char top; // screen top margin

    const char margin = 14;
    const char delSpaces = 40 - margin;

    cg_titlec(8, 5, 0, "New Guild Member");

    slot = nextFreeGuildSlot();
    if (slot == -1)
    {
        textcolor(2);
        puts("\nSorry, the guild is full."
             "\nPlease purge some inactive members"
             "before creating new ones.\n\n--key--");
        cgetc();
        return;
    }

    top = 5;
    newC = &guild[slot];

    cputsxy(2, top, "      Race:");
    for (i = 0; i < NUM_RACES; i++)
    {
        gotoxy(margin, top + i);
        cprintf("%d - %s", i + 1, gRaces[i]);
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
        gotoxy(margin, top + i);
        cprintf("%d - %s", i + 1, gClasses[i]);
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
            current = 7 + (rand() % 12) + gRaceModifiers[race][i];
            tempAttr[i] = current;
            cputsxy(margin, top + i, gAttributes[i]);
            gotoxy(margin + 13, top + i);
            cprintf("%2d %s", current, bonusStrForAttribute(current));
        }
        tempHP = 3 + (rand() % 8) + bonusValueForAttribute(tempAttr[0]);
        tempMP = 3 + (rand() % 8) + bonusValueForAttribute(tempAttr[1]);

        gotoxy(margin, top + i + 1);
        cprintf("Hit points   %2d", tempHP);

        gotoxy(margin, top + i + 2);
        cprintf("Magic points %2d", tempMP);

        cputsxy(margin, top + i + 4, "k)eep, r)eroll or q)uit? ");
        do
        {
            c = cgetc();
        } while (!strchr("rkq", c));
    } while (c == 'r');

    if (c == 'q')
        return;

    top = top + i + 4;
    cclearxy(0, top, 40);
    cputsxy(18, top + 1, "----------------");
    cputsxy(2, top, "Character name: ");
    fgets(cname, 16, stdin);

    // copy temp char to guild
    newC->city = city;
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

signed char nextFreePartySlot(void)
{
    signed char idx = -1;
    while (++idx < PARTYSIZE)
    {
        if (party[idx] == NULL)
        {
            return idx;
        }
    }
    return -1;
}

signed char nextFreeGuildSlot(void)
{
    signed char idx = -1;
    while (++idx < GUILDSIZE)
    {
        if (guild[idx].status == deleted)
        {
            return idx;
        }
    }
    return -1;
}

void saveGuild(void)
{
    static FILE *outfile;
    clrscr();
    cg_borders();
    puts("Saving...");
    outfile = fopen("guild", "w");
    fwrite(guild, GUILDSIZE * sizeof(character), 1, outfile);
    fclose(outfile);
    puts("done.");
    cgetc();
}

void loadGuild(void)
{
    static FILE *infile;
    infile = fopen("guild", "r");
    if (!infile)
    {
        return;
    }
    fread(guild, GUILDSIZE * sizeof(character), 1, infile);
    fclose(infile);
}

void initGuild(void)
{
    initGuildMem();
    loadGuild();
}

void initGuildMem(void)
{
    static unsigned int sizeBytes = 0;
    sizeBytes = GUILDSIZE * sizeof(character);
    guild = (character *)malloc(sizeBytes);
    if (guild == NULL)
    {
        puts("???fatal: no memory for guild");
        exit(0);
    }
    bzero(guild, sizeBytes);
}

signed char bonusValueForAttribute(attrT a)
{
    return -3 + (a / 3);
}

char *bonusStrForAttribute(attrT a)
{
    static char ret[6];
    signed char b = bonusValueForAttribute(a);
    strcpy(ret, "     ");
    if (b > 0)
    {
        sprintf(ret, "(+%d)", b);
    }
    else if (b < 0)
    {
        sprintf(ret, "(%d)", b);
    }
    return ret;
}