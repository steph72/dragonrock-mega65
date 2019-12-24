#ifndef _characterH
#define _characterH

#include "types.h"
#include "config.h"

byte loadParty(void);

extern character *party[PARTYSIZE];

char *nameOfInventoryItem(item *anItem);
char *nameOfInventoryItemWithID(itemT anItemID);
item *inventoryItemForID(itemT anItemID);
itemT addInventoryItem(itemT anItemID, character *aCharacter);
byte hasInventoryItem(character *aCharacter, itemT anItemID);
byte nextFreeInventorySlot(character *aCharacter);
byte partyMemberCount(void);

signed char bonusValueForAttribute(attrT a);
char* bonusStrForAttribute(attrT a);

item* getWeapon(character *aCharacter);

int getArmorClassForCharacter(character *aCharacter);
int getHitDiceForCharacter(character *aCharacter);

void showCurrentParty(byte small);
void displayInventoryAtRow(character *ic, byte row, char firstChar);
void inspectCharacter(byte idx);

item *whichItem(character *ic, byte *inventorySlot, byte *equipmentSlot);
void useItem(character *ic);

#endif