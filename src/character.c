#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"
#include "config.h"
#include "congui.h"
#include "types.h"

character *party[PARTYSIZE];

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

item* getWeapon(character *aCharacter) {
    if (aCharacter->weapon) {
        return inventoryItemForID(aCharacter->weapon);
    } else {
        return NULL;
    }
}

item* getArmor(character *aCharacter) {
    if (aCharacter->armor) {
        return inventoryItemForID(aCharacter->armor);
    } else {
        return NULL;
    }
}

item* getShield(character *aCharacter) {
    if (aCharacter->shield) {
        return inventoryItemForID(aCharacter->shield);
    } else {
        return NULL;
    }
}

int getArmorClassForCharacter(character *aCharacter) {
    int retAC = 10;
    item *armor;
    item *shield;

    retAC -= bonusValueForAttribute(aCharacter->attributes[aDEX]);
    armor = getArmor(aCharacter);
    shield = getShield(aCharacter);
    if (armor) {
        retAC -= armor->val1;
    }
    if (shield) {
        retAC -= shield->val1;
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
                *drbuf= 0;
                strncat(drbuf, c->name, 12);
                printf("%d %s", i + 1, drbuf);
            } else {
                printf("%d %s", i + 1, c->name);
            }
            if (!small) {
                cputsxy(20, y, gRacesS[c->aRace]);
                cputsxy(24, y, gClassesS[c->aClass]);
            }
            cputsxy(34, y, gStateDesc[c->status]);
        }
    }
}

void useSpecial(item *anItem) {
    if (anItem->id == 0) {
        cg_clearLower(3);
        gotoxy(0, 22);
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

item *whichItem(character *ic) {
    item *anItem;
    byte itemIdxChar;

    cputs("which item (A-L)? ");
    cursor(1);
    itemIdxChar= cgetc();
    cursor(0);
    anItem= inventoryItemForID(ic->inventory[itemIdxChar - 'a']);
    return anItem;
}

void useItem(character *ic) {
    item *anItem;
    cg_clearLower(3);
    gotoxy(0, 22);
    cputs("use ");
    anItem= whichItem(ic);

    switch (anItem->type) {

    case it_special:
        useSpecial(anItem);
        break;

    case it_scroll:
        useScroll(anItem);

    default:
        break;
    }
}

void inspectCharacter(byte idx) {
    character *ic;
    byte i;
    byte quitInspect;
    byte cmd;

    if (party[idx] == NULL) {
        return;
    }

    quitInspect= false;

    while (!quitInspect) {

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
        puts("Inventory:");
        for (i= 0; i < INV_SIZE; i++) {
            gotoxy(20 * (i / (INV_SIZE / 2)), 15 + (i % (INV_SIZE / 2)));
            printf("%c : %s", 'A' + i,
                   nameOfInventoryItemWithID(ic->inventory[i]));
        }
        gotoxy(0, 22);
        revers(1);
        cputs("u");
        revers(0);
        cputs("se/ready ");
        revers(1);
        cputs("r");
        revers(0);
        cputs("emove ");
        revers(1);
        cputs("g");
        revers(0);
        cputs("ive ");
        if (gCurrentGameMode == gm_city) {
            revers(1);
            cputs("s");
            revers(0);
            cputs("ell ");
        }
        revers(1);
        cputs("q");
        revers(0);
        cputs("uit");
        cputs(">");
        cursor(1);
        cmd= cgetc();
        cursor(0);

        if (cmd == 'q') {
            quitInspect= true;
        } else if (cmd == 'u') {
            useItem(ic);
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

    count = fgetc(infile);

    for (i=0;i<count;++i) {
        cputs(".");
        newChar = malloc(sizeof(character));
        fread(newChar,sizeof(character),1,infile);
        party[i]=newChar;
    }

    fclose(infile);
    return true;
}
