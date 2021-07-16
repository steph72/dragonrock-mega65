#include <c64.h>
//#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"
#include "congui.h"
#include "globals.h"
#include "memory.h"
#include "spell.h"

#define ITEM_HEADER_SIZE 0x08

character **party= (character **)0x5b0;
char *tempItemDesc= (char *)0x5d0;

long int gPartyGold;
long int gPartyExperience;

item tempItem;

item *inventoryItemForID(itemT anItemID) {
    static byte i;
    himemPtr ext;
    for (i= 0; i < 255; i++) {
        ext= ITEM_BASE + ITEM_HEADER_SIZE + (sizeof(item) * i);
        lcopy(ext, (long)&tempItem, sizeof(item));
        if (tempItem.id == anItemID) {
            return &tempItem;
        }
    }
    return NULL;
}

char *rawNameOfInventoryItem(item *anItem) {
    himemPtr ext;
    ext= ITEM_BASE + anItem->namePtr;
    lcopy(ext, (long)tempItemDesc, 32);
    return tempItemDesc;
}

char *nameOfInventoryItem(item *anItem) {
    char *rawName;
    rawName= rawNameOfInventoryItem(anItem);

    if (anItem->type == it_scroll) {
        sprintf(drbuf, "%s %d", rawName, anItem->val1);
        return drbuf;
    }
    if (anItem->val3 > 0) {
        sprintf(drbuf, "%s +%d", rawName, anItem->val3);
        return drbuf;
    }
    return rawName;
}

byte hasInventoryItem(character *aCharacter, itemT anItemID) {
    static byte i;
    for (i= 0; i < INV_SIZE; ++i) {
        if (aCharacter->inventory[i] == anItemID) {
            return true;
        }
    }
    return false;
}

byte nextFreeInventorySlot(character *aCharacter) {
    static byte i;
    for (i= 0; i < INV_SIZE; ++i) {
        if (!aCharacter->inventory[i]) {
            return i;
        }
    }
    return 0xff;
}

byte addInventoryItem(byte anItemID, character *aCharacter) {
    static byte i;
    i= nextFreeInventorySlot(aCharacter);
    if (i != 0xff) {
        aCharacter->inventory[i]= anItemID;
        return anItemID;
    }
    return 0;
}

void debugAddItem(character *aCharacter) {
    byte itemID;
    cg_puts("ADD:");
    fgets(drbuf, 3, stdin);
    itemID= atoi(drbuf);
    cg_printf("added %d", addInventoryItem(itemID, aCharacter));
}

signed char bonusValueForAttribute(attrT a) { return -3 + (a / 3); }

byte getNumberOfAttacks(character *aCharacter) {
    // TODO
    return 1;
}

int getHitDiceForCharacter(character *aCharacter) {
    item *weapon;
    if (aCharacter->weapon) {
        weapon= getWeapon(aCharacter);
        return weapon->val2;
    }
    return 3;
}

int getArmorClassForCharacter(character *aCharacter) {
    int retAC= 10;
    item *armor;
    item *shield;

    retAC-= bonusValueForAttribute(aCharacter->attributes[aDEX]);
    armor= getArmor(aCharacter);
    shield= getShield(aCharacter);
    if (armor) {
        retAC-= armor->val1;
    }
    if (shield) {
        retAC-= shield->val1;
    }
    // todo: test for rings etc.
    return retAC;
}

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
    static byte i;
    byte n= 0;
    for (i= 0; i < PARTYSIZE; ++i) {
        if (party[i]) {
            ++n;
        }
    }
    return n;
}

void showCurrentParty(byte startX, byte startY, byte small) {
    static byte i, x, y;
    character *c;

    y= startY;

    if (small) {
        x= 18;
        cg_textcolor(COLOR_YELLOW);
        cg_putsxy(18, 1, "#");
        cg_putsxy(20, 1, "Name");
        cg_putsxy(33, 1, "Status");
        cg_textcolor(COLOR_GRAY2);
        cg_hlinexy(17, 2, 38, 1);
        cg_hlinexy(17, 3 + partyMemberCount(), 38, 1);
    } else {
        x= startX;
        cg_textcolor(COLOR_YELLOW);
        cg_putsxy(17, y, "MP");
        cg_putsxy(25, y, "HP");
        cg_putsxy(3, y, "Name");
        cg_putsxy(1, y, "#");
        cg_putsxy(33, y, "Status");
    }

    for (i= 0; i < PARTYSIZE; i++) {
        if (party[i]) {
            c= party[i];
            ++y;
            cg_gotoxy(x, y);
            if (c->status == asleep) {
                cg_textcolor(COLOR_BLUE);
            }
            if (c->aHP >= 1) {
                cg_textcolor(COLOR_GREEN);
            } else {
                cg_textcolor(COLOR_RED);
            }
            if (small) {
                *drbuf= 0;
                strncat(drbuf, c->name, 12);
                cg_printf("%d %s", i + 1, drbuf);
            } else {
                cg_printf("%d %s", i + 1, c->name);
            }
            if (!small) {
                cg_gotoxy(17, y);
                cg_printf("%d/%d", c->aMP, c->aMaxMP);
                cg_gotoxy(25, y);
                cg_printf("%d/%d", c->aHP, c->aMaxHP);
            }
            cg_putsxy(35, y, gStateDesc[c->status]);
        }
    }
}

void useSpecial(item *anItem) {
    if (anItem->id == 0) {
        cg_clearLower(2);
        cg_gotoxy(0, 23);
        cg_puts("\r\nCurious. Nothing happens.\n--key--");
        cg_getkey();
    }
}

void more(char *filename) {
    FILE *infile;
    int line= 0;
    cg_clrscr();
    infile= fopen(filename, "r");
    while (!feof(infile)) {
        fgets(drbuf, DRBUFSIZE, infile);
        ++line;
        fputs(drbuf, stdout);
        if (line == gScreenRows - 2) {
            cg_gotoxy(28, gScreenRows - 1);
            cg_puts("-- more --");
            cg_cursor(1);
            cg_getkey();
            cg_cursor(0);
            line= 0;
            cg_clrscr();
        }
    }
    fclose(infile);
    cg_gotoxy(28, gScreenRows - 1);
    cg_puts("-- key --");
    cg_getkey();
}

void useScroll(item *anItem) {
    static byte num;
    num= anItem->val1;
    sprintf(drbuf, "fmsg%02d", num);
    more(drbuf);
}

item *getEquippedItem(character *ic, byte itemIdxChar, byte *equipSlot) {
    *equipSlot= itemIdxChar - 'a';
    switch (*equipSlot) {
    case 0:
        return inventoryItemForID(ic->weapon);
        break;

    case 1:
        return inventoryItemForID(ic->armor);
        break;

    case 2:
        return inventoryItemForID(ic->shield);
        break;

    default:
        break;
    }
    return NULL;
}

item *whichItem(character *ic, byte *inventorySlot, byte *equipSlot) {
    item *anItem;
    static byte itemIdxChar;

    *equipSlot= 255;
    *inventorySlot= 255;

    cg_puts("which item (A-O)? ");
    cg_cursor(1);
    do {
        itemIdxChar= cg_getkey();
    } while (itemIdxChar < 'a' || itemIdxChar > 'o');

    cg_cursor(0);
    if (itemIdxChar >= 'a' && itemIdxChar <= 'c') {
        anItem= getEquippedItem(ic, itemIdxChar, equipSlot);
        return anItem;
    }
    *inventorySlot= itemIdxChar - 'd';
    anItem= inventoryItemForID(ic->inventory[*inventorySlot]);
    return anItem;
}

void dispCharacterActionError(char *msg) {
    cg_clearLower(2);
    cg_gotoxy(0, 23);
    cg_textcolor(COLOR_LIGHTRED);
    cg_puts(msg);
    cg_puts("\n--key--");
    cg_textcolor(COLOR_WHITE);
    cg_getkey();
}

void giveItem(character *ic) {
    character *destCharacter;
    item *anItem;
    static byte inventorySlot;
    static byte equipSlot;
    static byte memberIdx;

    cg_clearLower(2);
    cg_gotoxy(0, 23);
    cg_puts("give ");
    anItem= whichItem(ic, &inventorySlot, &equipSlot);
    if (inventorySlot == 255) {
        dispCharacterActionError("unequip item first!");
        return;
    }
    cg_puts("\r\nto party member #");
    memberIdx= cg_getkey() - '1';
    if (memberIdx > 6 || party[memberIdx] == NULL) {
        dispCharacterActionError("...to whom?!");
        return;
    }
    destCharacter= party[memberIdx];
    if (!addInventoryItem(ic->inventory[inventorySlot], destCharacter)) {
        dispCharacterActionError("no space in inventory!");
        return;
    }
    ic->inventory[inventorySlot]= 0;
    return;
}

void removeItem(character *ic) {
    item *anItem;
    static byte equipmentSlot;
    static byte inventorySlot;
    cg_clearLower(2);
    cg_gotoxy(0, 23);
    cg_puts("remove ");
    anItem= whichItem(ic, &inventorySlot, &equipmentSlot);
    cg_clearLower(2);
    cg_gotoxy(0, 23);
    if (equipmentSlot == 255) {
        dispCharacterActionError("not equipped item!");
        return;
    }
    addInventoryItem(anItem->id, ic);
    switch (equipmentSlot) {
    case 0:
        ic->weapon= 0;
        break;

    case 1:
        ic->armor= 0;
        break;

    case 2:
        ic->shield= 0;

    default:
        break;
    }
}

void equipItem(item *anItem, byte inventorySlot, character *ic) {
    cg_clearLower(2);
    cg_gotoxy(0, 23);
    switch (anItem->type) {
    case it_weapon:
    case it_missile:
        if (ic->weapon != 0) {
            addInventoryItem(ic->weapon, ic);
        }
        ic->weapon= ic->inventory[inventorySlot];
        ic->inventory[inventorySlot]= 0;
        break;
    case it_armor:
        if (ic->armor != 0) {
            addInventoryItem(ic->armor, ic);
        }
        ic->armor= ic->inventory[inventorySlot];
        ic->inventory[inventorySlot]= 0;
        break;
    case it_shield:
        if (ic->shield != 0) {
            addInventoryItem(ic->armor, ic);
        }
        ic->shield= ic->inventory[inventorySlot];
        ic->inventory[inventorySlot]= 0;
        break;

    default:
        break;
    }
}

void useOrEquipItem(character *ic) {
    item *anItem;
    static byte equipmentSlot;
    static byte inventorySlot;
    cg_clearLower(2);
    cg_gotoxy(0, 23);
    cg_puts("use ");
    anItem= whichItem(ic, &inventorySlot, &equipmentSlot);

    switch (anItem->type) {

    case it_special:
        useSpecial(anItem);
        break;

    case it_scroll:
        useScroll(anItem);
        break;

    case it_armor:
    case it_missile:
    case it_shield:
    case it_weapon:
        if (equipmentSlot != 255) {
            dispCharacterActionError("Already equipped item!");
            return;
        }
        equipItem(anItem, inventorySlot, ic);
        break;

    default:
        break;
    }
}

void displayInventoryAtRow(character *ic, byte row, char firstChar) {
    static byte i;
    for (i= 0; i < INV_SIZE; i++) {
        cg_gotoxy(20 * (i / (INV_SIZE / 2)), row + (i % (INV_SIZE / 2)));
        cg_revers(1);
        cg_putc(firstChar + i);
        cg_revers(0);
        cg_putc(32);
        cg_puts(nameOfInventoryItemWithID(ic->inventory[i]));
    }
}

void inspectCharacter(byte idx) {
    character *ic;
    static byte i;
    static byte quitInspect;
    static byte cmd;

    static byte spellLine;

    if (party[idx] == NULL) {
        return;
    }

    quitInspect= false;

    while (!quitInspect) {

        spellLine= 0;
        ic= party[idx];
        cg_clrscr();
        if (gCurrentGameMode == gm_dungeon) {
            cg_textcolor(0);
        } else {
            cg_textcolor(5);
        }
        cg_revers(1);
        cg_puts(ic->name);
        cg_revers(0);
        cg_printf(" (%s, %s)\n", gRaces[ic->aRace], gClasses[ic->aClass]);
        for (i= 0; i < NUM_ATTRS; i++) {
            cg_putsxy(0, i + 2, gAttributesS[i]);
            cg_putsxy(3, i + 2, ":");
            cg_gotoxy(5, i + 2);
            cg_printf("%2d %s", ic->attributes[i],
                      bonusStrForAttribute(ic->attributes[i]));
        }
        cg_gotoxy(0, i + 3);
        cg_printf(" HP: %d/%d\n", ic->aHP, ic->aMaxHP);
        cg_printf(" MP: %d/%d\n", ic->aMP, ic->aMaxMP);
        cg_printf(" AC: %d", getArmorClassForCharacter(ic));
        cg_gotoxy(24, 2);
        cg_puts("Spells:");
        cg_gotoxy(24, 3);
        for (i= 1; i < 64; ++i) {
            if (hasSpell(ic, i)) {
                cg_printf("%2d ", i);
            }
            if (cg_wherex() >= 38) {
                ++spellLine;
                cg_gotoxy(24, 3 + spellLine);
            }
        }
        cg_gotoxy(13, 2);
        cg_printf("Age: %d", ic->age);
        cg_gotoxy(13, 3);
        cg_printf("Lvl: %d", ic->level);
        cg_gotoxy(13, 4);
        cg_printf(" XP: %d", ic->xp);
        cg_gotoxy(13, 5);
        cg_printf("Cns: %d", ic->gold);
        cg_gotoxy(16, 10);
        cg_revers(1);
        cg_putc('A');
        cg_revers(0);
        cg_printf(" Weapon: %s", nameOfInventoryItemWithID(ic->weapon));
        cg_gotoxy(16, 11);
        cg_revers(1);
        cg_putc('B');
        cg_revers(0);
        cg_printf("  Armor: %s", nameOfInventoryItemWithID(ic->armor));
        cg_gotoxy(16, 12);
        cg_revers(1);
        cg_putc('C');
        cg_revers(0);
        cg_printf(" Shield: %s", nameOfInventoryItemWithID(ic->shield));
        cg_gotoxy(0, 14);
        cg_puts("Inventory:");
        displayInventoryAtRow(ic, 16, 'D');
        cg_gotoxy(0, 23);
        cg_puts("u)se/ready r)emove g)ive ex)it\n>");
        cg_cursor(1);
        cmd= cg_getkey();
        cg_cursor(0);

        if (cmd >= '1' && cmd <= '6') {
            i= cmd - '1';
            if (party[i] != NULL) {
                idx= i;
            }
        }

        switch (cmd) {
        case 'x':
        case '\n':
        case '\r':
        case ' ':
            quitInspect= true;
            break;

        case 'u':
            useOrEquipItem(ic);
            break;

        case 'r':
            removeItem(ic);
            break;

        case 'g':
            giveItem(ic);
            break;

#ifdef DEBUG
        case 'D':
            debugAddItem(ic);
            break;
#endif

        default:
            break;
        }

    } // while !quitInspect
}

byte loadParty(void) {
    FILE *infile;
    static byte i, count;
    character *newChar;

    infile= fopen("pdata", "r");
    if (!infile) {
        return false;
    }

    count= fgetc(infile);

    for (i= 0; i < count; ++i) {
        newChar= malloc(sizeof(character));
        fread(newChar, sizeof(character), 1, infile);
        party[i]= newChar;
    }

    fclose(infile);
    return true;
}
