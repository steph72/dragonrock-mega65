
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
char nextFreeGuildSlot(void);
void newGuildMember(void);

void initGuild(void)
{
    initGuildMem();
}

void newGuildMember(void)
{
    byte i;
    byte race;
    byte class;

    cg_titlec(8, 5, 0, "New Guild Member");

    cputsxy(2, 3, " Race:");
    for (i = 0; i < NUM_RACES; i++)
    {
        cputcxy(9, 3 + i, '1' + i);
        cputsxy(10, 3 + i, " - ");
        cputs(gRaces[i]);
    }
    cputsxy(9, 4 + i, "Your choice: ");
    do
    {
        race = cgetc() - '1';
    } while (race >= NUM_RACES);
    for (i = 2; i < NUM_RACES + 6; cclearxy(9, ++i, 31))
        ;
    cputsxy(9, 3, gRaces[race]);

    cputsxy(2, 4, "Class:");
    for (i = 0; i < NUM_CLASSES; i++)
    {
        cputcxy(9, 4 + i, '1' + i);
        cputsxy(10, 4 + i, " - ");
        cputs(gClasses[i]);
    }
    cputsxy(9, 5 + i, "Your choice:");
    do
    {
        class = cgetc() - '1';
    } while (class >= NUM_CLASSES);
    for (i = 3; i < NUM_CLASSES + 7; cclearxy(9, ++i, 31))
        ;
    cputsxy(9, 4, gClasses[class]);

    cgetc();
}

char nextFreeGuildSlot(void)
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