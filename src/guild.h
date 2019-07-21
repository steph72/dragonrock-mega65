#ifndef __guild
#define __guild

#include "config.h"
#include "types.h"

void newGuildMember(byte city);
void listGuildMembers(void);
void purgeGuildMember(void);
void addToParty(void);
void dropFromParty(void);

byte isInParty(byte guildIdx);

signed char nextFreeGuildSlot(void);
signed char nextFreePartySlot(void);

void saveGuild(void);

extern character *party[PARTYSIZE];
extern character *guild;

#endif