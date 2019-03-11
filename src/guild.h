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

signed char nextFreeGuildSlot(void);
signed char nextFreePartySlot(void);

void saveGuild(void);

signed char bonusValueForAttribute(attrT a);
char* bonusStrForAttribute(attrT a);

extern character *party[PARTYSIZE];
extern character *guild;

#endif