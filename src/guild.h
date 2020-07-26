#ifndef __guild
#define __guild

#include "globals.h"

void newGuildMember(byte city);
void listGuildMembers(void);
void purgeGuildMember(void);
void addToParty(void);
void dropFromParty(void);

void setupGuildScreen();

byte isInParty(byte guildIdx);

signed char nextFreeGuildSlot(void);
signed char nextFreePartySlot(void);

void initGuildMem(void);
byte initGuild(void);
byte loadGuild(void);

void saveGuild(void);
void saveParty(void);

extern character *party[PARTYSIZE];
extern character *guild;

#endif