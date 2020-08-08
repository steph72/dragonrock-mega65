#include <c64.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"
#include "cityUI.h"
#include "congui.h"
#include "globals.h"
#include "memory.h"
#include "sprites.h"

#define SHOP_INV_SIZE 32
#define ITEMS_PER_PAGE 12
#define RESTOCK_FREQ 5 // restock every n visits

byte *shopInventory;
byte numCityVisits;

// clang-format off
#pragma code-name(push, "OVERLAY2");    // "CITY" segment
// clang-format on

void setupArmoryScreen() {
    setupCityScreen();
    setupCityScreen();
    revers(1);

    setSpriteEnabled(1, 1);
    putSprite(1, 268, 182);
    POKE(0xd01b, 0x0); // sprite prio high
    cg_setPalette(COLOR_PURPLE, 4, 1, 2);
    cg_block(gSecondaryAreaLeftX, gStatusAreaTopY, 39, 24, 160, COLOR_PURPLE);
    cg_block(0, 24, gMainAreaRightX, 24, 160, COLOR_PURPLE);
    sprintf(drbuf, "- %s armory -", gCities[gCurrentCityIndex]);
    textcolor(COLOR_PURPLE);
    cg_center(0, 24, gMainAreaWidth, drbuf);
}

byte currentShopSize(void) {
    byte i;
    for (i= 0; i < SHOP_INV_SIZE; ++i) {
        if (shopInventory[i] == NULL) {
            return i;
        }
    }
    return i;
}

item *itemAtRow(byte row) { return inventoryItemForID(shopInventory[row]); }

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
    restockItem(0x02, 4);
    restockItem(0x03, 5);
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

void sellItem(character *shopper) {
    byte val;
    byte slot;
    unsigned int price;
    item *anItem;
    byte sellQuit;
    sellQuit= false;
    do {
        cg_clearFromTo(3, 23);
        gotoxy(1, 4);
        puts("--- selling an item ---");
        gotoxy(0, 23);
        displayInventoryAtRow(shopper, 7, 'A');
        gotoxy(0, 20);
        cputs("Sell which item (x to abort) ");
        cursor(1);
        slot= cg_getkey();
        cursor(0);
        slot-= 'a';
        if (slot > INV_SIZE) {
            return;
        }
        if (shopper->inventory[slot] == 0) {
            return;
        }
        anItem= inventoryItemForID(shopper->inventory[slot]);
        price= anItem->price;
        gotoxy(0, 20);
        printf("\nSell %s for %u coins (y/n)?", anItem->name, price);
        cursor(1);
        do {
            val= cg_getkey();
        } while (val != 'y' && val != 'n');
        cputc(val);
        cursor(0);
        if (val != 'y') {
            return;
        }
        if (addItemIDToShopInventory(anItem->id)) {
            shopper->inventory[slot]= 0;
            cursor(1);
            cputs("\r\nSell another (y/n)? ");
            val= cg_getkey();
            sellQuit= (val == 'n');
        } else {
            puts("\nshop is full!\n--key--");
            cg_getkey();
            return;
        }
    } while (!sellQuit);
}

unsigned int salePrice(character *shopper, item *anItem) {
    unsigned long p;
    int charBonus;

    charBonus= bonusValueForAttribute(shopper->attributes[aCHR]);
    p= (anItem->price * (100UL + (10 * charBonus))) / 100UL;
    return p;
}

int chooseShopOrInventoryItem(character *shopper, byte buySellMode) {

    signed char offset, shopIndex;
    itemT invItemID;
    item *anItem;
    byte y, row, choice;
    byte stopsize;
    byte cmd;

    offset= -5;
    choice= 1;

    textcolor(COLOR_ORANGE);
    cputsxy(0, gMainAreaTopY, "Name              Price      ");

    textcolor(COLOR_GRAY3);

    do {

        if (buySellMode == 0) {
            stopsize= currentShopSize();
        } else {
            stopsize= inventoryCount(shopper);
        }

        for (row= 0; row < 14; ++row) {
            y= gMainAreaTopY + 1 + row;
            shopIndex= row + offset + choice - 1;
            cg_line(gMainAreaTopY + 1 + row, 0, gMainAreaRightX, 160,
                    COLOR_GRAY3);
            if (shopIndex == -1 || shopIndex == stopsize) {
                cputsxy(0, y, "(exit)");
            }
            if (shopIndex >= 0) {
                if (buySellMode == 0) {
                    if (shopInventory[shopIndex]) {
                        cputsxy(0, y, itemAtRow(shopIndex)->name);
                        gotoxy(18, y);
                        cprintf("%u", itemAtRow(shopIndex)->price);
                    }
                } else {
                    invItemID= shopper->inventory[shopIndex];
                    if (invItemID) {
                        anItem= inventoryItemForID(invItemID);
                        cputsxy(0, y, anItem->name);
                        gotoxy(18, y);
                        cprintf("%u", salePrice(shopper, anItem));
                    }
                }
            }
        }

        cg_line(gMainAreaTopY + 1 + 5, 0, gMainAreaRightX, 0, 1);

        /*
                gotoxy(0, 0);
                printf("%d %d", choice - 1, shopInventory[choice - 1]);
                */

        while (!kbhit()) {
            cg_stepColor();
        }
        cmd= cgetc();

        switch (cmd) {
        case 17: // down
            if (choice <= stopsize)
                choice++;
            break;

        case 145: // up
            if (choice > 0)
                choice--;
            break;

        case 29: // cursor right
            return 1001;
            break;

        case 157: // cursor left
            return 1000;
            break;

        default:
            break;
        }
    } while (cmd != 13);

    cg_line(gMainAreaTopY + 1 + 5, 0, gMainAreaRightX, 0, COLOR_GRAY1);

    if (choice == stopsize + 1) {
        choice= 0;
    }

    return choice;
}

void doArmory(void) {
    int cmd;
    byte currentShopperIdx;
    character *shopper;
    byte buySellMode;

    buySellMode= 0;
    currentShopperIdx= 0;

    do {

        setupArmoryScreen();
        clearPartyArea();
        textcolor(COLOR_GRAY2);
        revers(1);
        showCurrentParty(false, false);
        clearMenuArea();
        cputsxy(0, partyMemberCount(), "(leave armory)");
        textcolor(COLOR_CYAN);
        cg_center(gSecondaryAreaLeftX, gMenuAreaTopY + 1, gSecondaryAreaWidth,
                  "who wants");
        cg_center(gSecondaryAreaLeftX, gMenuAreaTopY + 2, gSecondaryAreaWidth,
                  "  to go  ");
        cg_center(gSecondaryAreaLeftX, gMenuAreaTopY + 3, gSecondaryAreaWidth,
                  "shopping?");

        cmd= cg_verticalChooser(0, 0, 1, 14, partyMemberCount() + 1,
                                currentShopperIdx);

        if (cmd == partyMemberCount()) {
            return;
        }

        currentShopperIdx= cmd;
        cg_block(0, cmd, 39, cmd, 0, COLOR_GREEN);

        do {

            cg_block(0, partyMemberCount(), 39, partyMemberCount(), 160,
                     COLOR_GRAY2);
            cg_block(0, 22, gMainAreaRightX, 24, 160, COLOR_GRAY1);
            textcolor(COLOR_GRAY1);
            cputsxy(5, 23, "buy");
            cputsxy(5 + gMainAreaWidth / 2, 23, "sell");
            if (buySellMode == 0) {
                cg_block(0, 22, (gMainAreaWidth / 2), 24, 0, COLOR_YELLOW);
            } else {
                cg_block(1 + (gMainAreaWidth / 2), 22, gMainAreaRightX, 24, 0,
                         COLOR_YELLOW);
            }

            clearMenuArea();

            shopper= party[currentShopperIdx];
            cmd= chooseShopOrInventoryItem(shopper, buySellMode);

            if (cmd == 1001 || cmd == 1000) {
                buySellMode= !buySellMode;
            }

        } while (cmd != 0);

    } while (1);

    /*
        do {
            cursor(0);
            sprintf(drbuf, "%s Armory", gCities[gCurrentCityIndex]);
            cg_titlec(COLOR_BLUE, COLOR_GREEN, 0, drbuf);
            gotoxy(0, 19);
            printf("%s coins: %d", shopper->name, shopper->gold);
            puts("\n\nA)-L) buy item  S)ell item  eX)it shop");
            // dispInvFromIndex(0);
            gotoxy(0, 22);
            cputs(">");
            cursor(1);
            cmd= cgetc();
            cursor(0);
            if (cmd == 's') {
                sellItem(shopper);
            }
        } while (cmd != 'x');
        */
}

// clang-format off
#pragma code-name(pop);    // "CITY" segment
// clang-format on