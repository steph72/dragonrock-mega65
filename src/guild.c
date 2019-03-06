
#include <stdio.h>
#include <stdlib.h>
#include "congui.h"
#include "types.h"
#include "config.h"
#include "debug.h"

character *guild;
character *party;

void initGuildMem(void);

void initGuild(void)
{
    initGuildMem();
}

void initGuildMem(void)
{
    int sizeBytes = 0;
    sizeBytes = GUILDSIZE * sizeof(character);
    guild = (character *)malloc(sizeBytes);
    if (guild == NULL)
    {
        cg_println("???fatal: no memory for guild");
        exit(0);
    }
    dbgMem(sizeBytes, guild, "guild");
    sizeBytes = PARTYSIZE * sizeof(character);
    party = (character *)malloc(sizeBytes);
    if (party == NULL)
    {
        cg_println("???fatal: no memory for party");
        exit(0);
    }
    dbgMem(sizeBytes, party, "party");
}