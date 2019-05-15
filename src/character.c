#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"

#include "config.h"
#include "types.h"

item *inventoryItemForID(itemT anItemID) {
    register byte i;
    item *anItem;
    for (i= 0; i < 255; i++) {
        anItem= &gItems[i];
        if (anItem->id == anItemID) {
            return anItem;
        }
    }
    return NULL;
}

char *nameOfInventoryItemWithID(itemT anItemID) {
    return nameOfInventoryItem(inventoryItemForID(anItemID));
}

char *nameOfInventoryItem(item *anItem) {
    if (!anItem) {
        return "--";
    }
    return anItem->name;
}

itemT addInventoryItem(itemT anItemID, character *aCharacter) {
    register byte i;
    for (i= 0; i < INV_SIZE; i++) {
        if (!aCharacter->inventory[i]) {
            aCharacter->inventory[i]= anItemID;
            return anItemID;
        }
    }
    return 0;
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