#ifndef _characterH
#define _characterH

#include "types.h"
#include "config.h"

extern character *party[PARTYSIZE];

char *nameOfInventoryItem(item *anItem);
char *nameOfInventoryItemWithID(itemT anItemID);
itemT addInventoryItem(itemT anItemID, character *aCharacter);
byte hasInventoryItem(character *aCharacter, itemT anItemID);
byte nextFreeInventorySlot(character *aCharacter);

signed char bonusValueForAttribute(attrT a);
char* bonusStrForAttribute(attrT a);

#endif