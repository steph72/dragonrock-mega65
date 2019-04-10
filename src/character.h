#ifndef _characterH
#define _characterH

#include "types.h"

char *nameOfInventoryItem(item *anItem);
char *nameOfInventoryItemWithID(itemT anItemID);

itemT addInventoryItem(itemT anItemID, character *aCharacter);

signed char bonusValueForAttribute(attrT a);
char* bonusStrForAttribute(attrT a);

#endif