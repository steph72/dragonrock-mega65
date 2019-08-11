#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"

#include "config.h"
#include "types.h"

extern char outbuf[80];

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

byte hasInventoryItem(character *aCharacter, itemT anItemID) {
    register byte i;
    for (i= 0; i < INV_SIZE; ++i) {
        if (aCharacter->inventory[i] == anItemID) {
            return true;
        }
    }
    return false;
}

byte nextFreeInventorySlot(character *aCharacter) {
    register byte i;
    for (i= 0; i < INV_SIZE; ++i) {
        if (!aCharacter->inventory[i]) {
            return i;
        }
    }
    return 0xff;
}

itemT addInventoryItem(itemT anItemID, character *aCharacter) {
    byte i;
    i= nextFreeInventorySlot(aCharacter);
    if (i != 0xff) {
        aCharacter->inventory[i]= anItemID;
        return anItemID;
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

byte partyMemberCount(void) {
    byte i;
    byte n=0;
    for (i=0;i<PARTYSIZE;++i) {
        if (party[i]) {
            ++n;
        }
    }
    return n;
}

void showCurrentParty(byte small) {
    static byte i, x, y;
    static character *c;

    y= 2;

    if (small) {
        x= 19;
    } else {
        x= 0;
    }

    for (i= 0; i < PARTYSIZE; i++) {
        if (party[i]) {
            c= party[i];
            ++y;
            gotoxy(x, y);
            if (small) {
                *outbuf= 0;
                strncat(outbuf, c->name, 12);
                cprintf("%d %s", i + 1, outbuf);
            } else {
                cprintf("%d %s", i + 1, c->name);
            }
            if (!small) {
                cputsxy(20, y, gRacesS[c->aRace]);
                cputsxy(24, y, gClassesS[c->aClass]);
            }
            cputsxy(34, y, gStateDesc[c->status]);
        }
    }
}

void inspectCharacter(byte idx) {
    character *ic;
    byte i;
    if (party[idx] == NULL) {
        return;
    }
    ic= party[idx];
    clrscr();
    revers(1);
    cputs(ic->name);
    revers(0);
    printf(" (%s, %s)\n", gRaces[ic->aRace], gClasses[ic->aClass]);
    for (i= 0; i < strlen(ic->name); ++i) {
        cputc('=');
    }
    puts("\n");
    for (i= 0; i < NUM_ATTRS; i++) {
        cputsxy(0, i + 3, gAttributesS[i]);
        cputsxy(3, i + 3, ":");
        gotoxy(5, i + 3);
        cprintf("%2d %s", ic->attributes[i],
                bonusStrForAttribute(ic->attributes[i]));
    }
    gotoxy(0, i + 4);
    printf(" HP: %d/%d\n", ic->aHP, ic->aMaxHP);
    printf(" MP: %d/%d\n", ic->aMP, ic->aMaxMP);
    gotoxy(16, 3);
    printf("   Age: %d", ic->age);
    gotoxy(16, 4);
    printf(" Level: %d", ic->level);
    gotoxy(16, 5);
    printf("    XP: %d", ic->xp);
    gotoxy(16, 6);
    printf(" Coins: %d", ic->gold);
    gotoxy(16, 8);
    printf("Weapon: %s", nameOfInventoryItemWithID(ic->weapon));
    gotoxy(16, 9);
    printf(" Armor: %s", nameOfInventoryItemWithID(ic->armor));
    gotoxy(16, 10);
    printf("Shield: %s", nameOfInventoryItemWithID(ic->shield));
    gotoxy(0, 13);
    printf("Inventory:\n");
    for (i= 0; i < INV_SIZE; i++) {
        gotoxy(20 * (i / (INV_SIZE / 2)), 15 + (i % (INV_SIZE / 2)));
        printf("%c : %s", 'A' + i, nameOfInventoryItemWithID(ic->inventory[i]));
    }
    cgetc();
}
