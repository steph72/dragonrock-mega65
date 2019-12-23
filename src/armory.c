#include <conio.h>
#include <plus4.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"
#include "config.h"
#include "congui.h"
#include "types.h"

#define SHOP_INV_SIZE 32
#define ITEMS_PER_PAGE 12
#define RESTOCK_FREQ 5

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
        gotoxy(3, 3 + i);
        if (shopInventory[itemIdx]) {
            anItem= inventoryItemForID(shopInventory[itemIdx]);
            printf("%c %-10s %4d", 'A' + i, anItem->name, anItem->price);
        }
    }
}

void doArmory(void) {
    char cmd;
    character *shopper;
    cursor(1);
    cputsxy(0, 21, "Who wants to go shopping? ");
    cmd= cgetc();
    if (cmd < '1' || cmd > '6') {
        return;
    }
    cmd-= '1';
    if (party[cmd] == NULL) {
        return;
    }

    shopper= party[cmd];

    do {
        cursor(0);
        sprintf(drbuf, "%s Armory", gCities[gCurrentCityIndex]);
        cg_titlec(BCOLOR_BLUE | CATTR_LUMA3, BCOLOR_GREEN | CATTR_LUMA5, 0,
                  drbuf);
        gotoxy(0, 19);
        printf("%s coins: %d", shopper->name, shopper->gold);
        puts("\n\nA)-L) buy item  S)ell item  eX)it shop");
        dispInvFromIndex(0);
        gotoxy(0, 22);
        cputs(">");
        cursor(1);
        cmd= cgetc();
        cursor(0);
    } while (cmd != 'x');
}

// clang-format off
#pragma code-name(pop);    // "CITY" segment
// clang-format on