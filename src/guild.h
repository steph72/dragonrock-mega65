#ifndef __guild
#define __guild

#include "config.h"

void initGuild(void);
void newGuildMember(byte city);
void listGuildMembers(void);
void purgeGuildMember(void);
void addToParty(void);

void saveGuild(void);
signed char bonusValueForAttribute(attrT a);
char* bonusStrForAttribute(attrT a);

extern character *party[PARTYSIZE];

#endif