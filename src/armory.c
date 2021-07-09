//#include <conio.h>
#include <c64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "character.h"
#include "congui.h"

#define SHOP_INV_SIZE 32
#define ITEMS_PER_PAGE 12
#define RESTOCK_FREQ 5          // restock every n visits

byte *shopInventory;
byte numCityVisits;

// clang-format off
#pragma code-name(push, "OVERLAY2");    // "CITY" segment
// clang-format on


byte addItemIDToShopInventory(byte itemID) {
    byte i;
    for (i= 0; i < SHOP_INV_SIZE; ++i) {
        if (!shopInventory[i]) {
            shopInventory[i]= itemID;
            return true;
        }
    }
    return false;
}

byte numberOfItemsWithID(byte itemID) {
    byte i;
    byte count= 0;
    for (i= 0; i < SHOP_INV_SIZE; ++i) {
        if (shopInventory[i] == itemID) {
            ++count;
        }
    }
    return count;
}

void restockItem(byte itemID, byte count) {
    byte currentCount;
    byte toAdd;
    currentCount= numberOfItemsWithID(itemID);
    if (currentCount >= count) {
        return;
    }
    toAdd= count - currentCount;
    while (toAdd > 0) {
        addItemIDToShopInventory(itemID);
        toAdd--;
    }
}

void restockShop(void) {
    restockItem(0x01, 3);
    restockItem(0x02, 3);
    restockItem(0x03, 3);
}

void releaseArmory(void) { free(shopInventory); }

void initArmory() {
    FILE *shopInvFile;
    numCityVisits= 0;
    sprintf(drbuf, "s%d", gCurrentCityIndex);
    shopInventory= (byte *)malloc(SHOP_INV_SIZE);
    shopInvFile= fopen(drbuf, "rb");
    if (shopInvFile) {
        numCityVisits= fgetc(shopInvFile);
        fread(shopInventory, SHOP_INV_SIZE, 1, shopInvFile);
        fclose(shopInvFile);
    } else {
        memset(shopInventory, 0, SHOP_INV_SIZE);
    }
    if (numCityVisits % RESTOCK_FREQ == 0) {
        restockShop();
    }
    numCityVisits++;
}

void saveArmory() {
    FILE *shopInvFile;
    sprintf(drbuf, "s%d", gCurrentCityIndex);
    shopInvFile= fopen(drbuf, "wb");
    fputc(numCityVisits, shopInvFile);
    fwrite(shopInventory, SHOP_INV_SIZE, 1, shopInvFile);
    fclose(shopInvFile);
}

void dispInvFromIndex(byte idx) {
    byte i;
    byte itemIdx;
    item *anItem;
    for (i= 0; i < ITEMS_PER_PAGE; ++i) {
        itemIdx= idx + i;
        if (itemIdx >= SHOP_INV_SIZE) {
            return;
        }
        cg_gotoxy(3, 3 + i);
        if (shopInventory[itemIdx]) {
            anItem= inventoryItemForID(shopInventory[itemIdx]);
            printf("%c %-10s %5u", 'A' + i, nameOfInventoryItem(anItem), anItem->price);
        }
    }
}

unsigned int salePrice(character *shopper, item *anItem) {
    unsigned long p;
    signed char charBonus;

    charBonus= bonusValueForAttribute(shopper->attributes[aCHR]);
    p= (anItem->price * (100UL + (10 * charBonus))) / 100UL;
    return p;
}


void sellItem(character *shopper) {
    byte val;
    byte slot;
    unsigned int price;
    item *anItem;
    byte sellQuit;
    sellQuit= false;
    do {
        cg_clearFromTo(3, 23);
        cg_gotoxy(1, 4);
        cg_puts("--- selling an item ---");
        cg_gotoxy(0, 23);
        displayInventoryAtRow(shopper, 7, 'A');
        cg_gotoxy(0, 20);
        cg_puts("Sell which item (x to abort) ");
        cg_cursor(1);
        slot= cg_getkey();
        cg_cursor(0);
        slot-= 'a';
        if (slot > INV_SIZE) {
            return;
        }
        if (shopper->inventory[slot] == 0) {
            return;
        }
        anItem= inventoryItemForID(shopper->inventory[slot]);
        price= anItem->price;
        cg_gotoxy(0, 20);
        cg_printf("\nSell %s for %u coins (y/n)?", nameOfInventoryItem(anItem), price);
        cg_cursor(1);
        do {
            val= cg_getkey();
        } while (val != 'y' && val != 'n');
        cg_putc(val);
        cg_cursor(0);
        if (val!='y') {
            return;
        }
        if (addItemIDToShopInventory(anItem->id)) {
            shopper->inventory[slot] = 0;
            cg_cursor(1);
            cg_puts("\r\nSell another (y/n)? ");
            val=cg_getkey();
            sellQuit = (val=='n');
        } else {
            cg_puts("\nshop is full!\n--key--");
            cg_getkey();
            return;
        }
    } while (!sellQuit);
}

void doArmory(void) {
    char cmd;
    character *shopper;
    cg_cursor(1);
    cg_putsxy(0, 21, "Who wants to go shopping? ");
    cmd= cg_getkey();
    if (cmd < '1' || cmd > '6') {
        return;
    }
    cmd-= '1';
    if (party[cmd] == NULL) {
        return;
    }

    shopper= party[cmd];

    do {
        cg_cursor(0);
        sprintf(drbuf, "%s Armory", gCities[gCurrentCityIndex]);
        cg_titlec(COLOR_GREEN, 0,
                  drbuf);
        cg_gotoxy(0, 19);
        cg_printf("%s coins: %d", shopper->name, shopper->gold);
        cg_puts("\n\nA)-L) buy item  S)ell item  eX)it shop");
        dispInvFromIndex(0);
        cg_cursor(1);
        cmd= cg_getkeyP(0,22,">");
        cg_cursor(0);
        if (cmd == 's') {
            sellItem(shopper);
        }
    } while (cmd != 'x');
}

// clang-format off
#pragma code-name(pop);    // "CITY" segment
// clang-format on