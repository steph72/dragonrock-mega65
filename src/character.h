#ifndef _characterH
#define _characterH

#include "types.h"

char *nameOfInventoryItem(item *anItem);
item* addInventoryItem(item *anItem, character *aCharacter);
item* addInventoryItemByID(byte itemID, character *aCharacter);

signed char bonusValueForAttribute(attrT a);
char* bonusStrForAttribute(attrT a);

#endif