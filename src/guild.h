#ifndef __guild
#define __guild

#include "config.h"
#include "types.h"

byte initGuild(void);
void newGuildMember(byte city);
void listGuildMembers(void);
void purgeGuildMember(void);
void addToParty(void);
void dropFromParty(void);
void showCurrentParty(void);

byte isInParty(byte guildIdx);

signed char nextFreeGuildSlot(void);
signed char nextFreePartySlot(void);

void saveGuild(void);

extern character *party[PARTYSIZE];
extern character *guild;

#endif