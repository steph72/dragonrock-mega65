#include <conio.h>
#include <plus4.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"
#include "congui.h"
#include "spell.h"
#include "globals.h"

character *party[PARTYSIZE];

long int gPartyGold;
long int gPartyExperience;

item *inventoryItemForID(itemT anItemID) {
    register byte i;
    item *anItem;
    for (i= 0; i < 255; i++) {
        anItem= &gItems[i];
        if (anItem->id == anItemID) {
            return anItem;
        }
        if (anItem->id == 255) {
            break;
        }
    }
    return NULL;
}

char *nameOfInventoryItemWithID(itemT anItemID) {
    return nameOfInventoryItem(inventoryItemForID(anItemID));
}

char *nameOfInventoryItem(item *anItem) {
    if (anItem->type == it_scroll) {
        sprintf(drbuf, "%s %d", anItem->name, anItem->val1);
        return drbuf;
    }
    if (anItem->val3 > 0) {
        sprintf(drbuf, "%s +%d", anItem->name, anItem->val3);
        return drbuf;
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

byte addInventoryItem(byte anItemID, character *aCharacter) {
    byte i;
    i= nextFreeInventorySlot(aCharacter);
    if (i != 0xff) {
        aCharacter->inventory[i]= anItemID;
        return anItemID;
    }
    return 0;
}

signed char bonusValueForAttribute(attrT a) { return -3 + (a / 3); }

item *getWeapon(character *aCharacter) {
    return inventoryItemForID(aCharacter->weapon);
}

item *getArmor(character *aCharacter) {
    return inventoryItemForID(aCharacter->armor);
}

item *getShield(character *aCharacter) {
    return inventoryItemForID(aCharacter->shield);
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
    byte i;
    byte n= 0;
    for (i= 0; i < PARTYSIZE; ++i) {
        if (party[i]) {
            ++n;
        }
    }
    return n;
}

void showCurrentParty(byte small) {
    static byte i, x, y;
    static character *c;

    y= 3;

    if (small) {
        x= 19;
    } else {
        x= 0;
        cputsxy(17,2,"MP");
        cputsxy(25,2,"HP");
        cputsxy(2,2,"Name");
        cputsxy(0,2,"#");
        cputsxy(33,2,"Status");
    }


    for (i= 0; i < PARTYSIZE; i++) {
        if (party[i]) {
            c= party[i];
            ++y;
            gotoxy(x, y);
            if (small) {
                *drbuf= 0;
                strncat(drbuf, c->name, 12);
                printf("%d %s", i + 1, drbuf);
            } else {
                printf("%d %s", i + 1, c->name);
            }
            if (!small) {
                gotoxy(17,y);
                cprintf("%d/%d",c->aMP,c->aMaxMP);
                gotoxy(25,y);
                cprintf("%d/%d",c->aHP,c->aMaxHP);
            }
            cputsxy(33, y, gStateDesc[c->status]);
        }
    }
}

void useSpecial(item *anItem) {
    if (anItem->id == 0) {
        cg_clearLower(2);
        gotoxy(0, 23);
        cputs("\r\nCurious. Nothing happens.\r\n--key--");
        cgetc();
    }
}

void more(char *filename) {
    FILE *infile;
    int line= 0;
    clrscr();
    infile= fopen(filename, "r");
    while (!feof(infile)) {
        fgets(drbuf, BUFSIZE, infile);
        ++line;
        fputs(drbuf, stdout);
        if (line == 23) {
            gotoxy(28, 24);
            cputs("-- more --");
            cursor(1);
            cgetc();
            cursor(0);
            line= 0;
            clrscr();
        }
    }
    fclose(infile);
    gotoxy(28, 24);
    cputs("-- key --");
    cgetc();
}

void useScroll(item *anItem) {
    byte num;
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
    byte itemIdxChar;

    *equipSlot= 255;
    *inventorySlot= 255;

    cputs("which item (A-O)? ");
    cursor(1);
    do {
        itemIdxChar= cgetc();
    } while (itemIdxChar < 'a' || itemIdxChar > 'o');

    cursor(0);
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
    gotoxy(0, 23);
    textcolor(BCOLOR_RED | CATTR_LUMA3);
    cputs(msg);
    cputs("\r\n--key--");
    textcolor(BCOLOR_WHITE | CATTR_LUMA7);
    cg_getkey();
}

void giveItem(character *ic) {
    character *destCharacter;
    item *anItem;
    byte inventorySlot;
    byte equipSlot;
    byte memberIdx;

    cg_clearLower(2);
    gotoxy(0, 23);
    cputs("give ");
    anItem= whichItem(ic, &inventorySlot, &equipSlot);
    if (inventorySlot == 255) {
        dispCharacterActionError("unequip item first!");
        return;
    }
    cputs("\r\nto party member #");
    memberIdx= cgetc() - '1';
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
    byte equipmentSlot;
    byte inventorySlot;
    cg_clearLower(2);
    gotoxy(0, 23);
    cputs("remove ");
    anItem= whichItem(ic, &inventorySlot, &equipmentSlot);
    cg_clearLower(2);
    gotoxy(0, 23);
    if (equipmentSlot == 255) {
        dispCharacterActionError("not equipped item!");
        return;
    }
    addInventoryItem(anItem->id, ic);
    switch (equipmentSlot) {
    case 0:
        ic->weapon= NULL;
        break;

    case 1:
        ic->armor= NULL;
        break;

    case 2:
        ic->shield= NULL;

    default:
        break;
    }
}

void equipItem(item *anItem, byte inventorySlot, character *ic) {
    cg_clearLower(2);
    gotoxy(0, 23);
    switch (anItem->type) {
    case it_weapon:
    case it_missile:
        if (ic->weapon != NULL) {
            addInventoryItem(ic->weapon, ic);
        }
        ic->weapon= ic->inventory[inventorySlot];
        ic->inventory[inventorySlot]= NULL;
        break;
    case it_armor:
        if (ic->armor != NULL) {
            addInventoryItem(ic->armor, ic);
        }
        ic->armor= ic->inventory[inventorySlot];
        ic->inventory[inventorySlot]= 0;
        break;
    case it_shield:
        if (ic->shield != NULL) {
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
    byte equipmentSlot;
    byte inventorySlot;
    cg_clearLower(2);
    gotoxy(0, 23);
    cputs("use ");
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
        equipItem(anItem, inventorySlot, ic);
        break;

    default:
        break;
    }
}

void displayInventoryAtRow(character *ic, byte row, char firstChar) {
    byte i;
    for (i= 0; i < INV_SIZE; i++) {
        gotoxy(20 * (i / (INV_SIZE / 2)), row + (i % (INV_SIZE / 2)));
        revers(1);
        cputc(firstChar + i);
        revers(0);
        cputc(32);
        cputs(nameOfInventoryItemWithID(ic->inventory[i]));
    }
}

void inspectCharacter(byte idx) {
    character *ic;
    byte i;
    byte quitInspect;
    byte cmd;

    byte spellLine;

    if (party[idx] == NULL) {
        return;
    }

    quitInspect= false;

    while (!quitInspect) {

        spellLine= 0;
        ic= party[idx];
        clrscr();
        revers(1);
        cputs(ic->name);
        revers(0);
        printf(" (%s, %s)\n", gRaces[ic->aRace], gClasses[ic->aClass]);
        for (i= 0; i < NUM_ATTRS; i++) {
            cputsxy(0, i + 2, gAttributesS[i]);
            cputsxy(3, i + 2, ":");
            gotoxy(5, i + 2);
            cprintf("%2d %s", ic->attributes[i],
                    bonusStrForAttribute(ic->attributes[i]));
        }
        gotoxy(0, i + 3);
        printf(" HP: %d/%d\n", ic->aHP, ic->aMaxHP);
        printf(" MP: %d/%d\n", ic->aMP, ic->aMaxMP);
        printf(" AC: %d", getArmorClassForCharacter(ic));
        gotoxy(24, 2);
        cputs("Spells:");
        gotoxy(24, 3);
        for (i= 1; i < 64; ++i) {
            if (hasSpell(ic, i)) {
                printf("%2d ", i);
            }
            if (wherex() >= 38) {
                ++spellLine;
                gotoxy(24, 3 + spellLine);
            }
        }
        gotoxy(13, 2);
        printf("Age: %d", ic->age);
        gotoxy(13, 3);
        printf("Lvl: %d", ic->level);
        gotoxy(13, 4);
        printf(" XP: %d", ic->xp);
        gotoxy(13, 5);
        printf("Cns: %d", ic->gold);
        gotoxy(16, 10);
        revers(1);
        cputc('A');
        revers(0);
        printf(" Weapon: %s", nameOfInventoryItemWithID(ic->weapon));
        gotoxy(16, 11);
        revers(1);
        cputc('B');
        revers(0);
        printf("  Armor: %s", nameOfInventoryItemWithID(ic->armor));
        gotoxy(16, 12);
        revers(1);
        cputc('C');
        revers(0);
        printf(" Shield: %s", nameOfInventoryItemWithID(ic->shield));
        gotoxy(0, 14);
        puts("Inventory:");
        displayInventoryAtRow(ic,16,'D');
        gotoxy(0, 23);
        cputs("u)se/ready r)emove g)ive ex)it\r\n>");
        cursor(1);
        cmd= cgetc();
        cursor(0);

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

        default:
            break;
        }

    } // while !quitInspect
}

byte loadParty(void) {
    static FILE *infile;
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
