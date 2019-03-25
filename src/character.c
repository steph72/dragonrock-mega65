#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"

#include "config.h"
#include "types.h"

char *nameOfInventoryItem(item *anItem) {
    if (!anItem) {
        return "--";
    }
    return anItem->name;
}

item* addInventoryItem(item *anItem, character *aCharacter) {
    register byte i;
    for (i= 0; i < INV_SIZE; i++) {
        if (!aCharacter->inventory[i]) {
            aCharacter->inventory[i]= anItem;
            return anItem;
        }
    }
    return NULL;
}

item *addInventoryItemByID(byte itemID, character *aCharacter) {
    register byte i;
    item *anItem;
    for (i= 1; i < 255; i++) {
        anItem= &gItems[i];
        if (anItem->id == itemID) {
            return addInventoryItem(anItem, aCharacter);
        }
    }
    return NULL;
}

signed char bonusValueForAttribute(attrT a) { return -3 + (a / 3); }

char *bonusStrForAttribute(attrT a) {
    static char ret[6];
    signed char b= bonusValueForAttribute(a);
    strcpy(ret, "     ");
    if (b > 0) {
        sprintf(ret, "(+%d)", b);
    } else if (b < 0) {
        sprintf(ret, "(%d)", b);
    }
    return ret;
}