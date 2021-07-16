#ifndef _characterH
#define _characterH

#include "globals.h"



byte loadParty(void);

char *nameOfInventoryItem(item *anItem);
item *inventoryItemForID(itemT anItemID);
itemT addInventoryItem(itemT anItemID, character *aCharacter);
byte hasInventoryItem(character *aCharacter, itemT anItemID);
byte nextFreeInventorySlot(character *aCharacter);
byte partyMemberCount(void);

signed char bonusValueForAttribute(attrT a);
char* bonusStrForAttribute(attrT a);
byte getNumberOfAttacks(character *aCharacter);

item* getWeapon(character *aCharacter);

int getArmorClassForCharacter(character *aCharacter);
int getHitDiceForCharacter(character *aCharacter);

void showCurrentParty(byte startX, byte startY, byte small);
void displayInventoryAtRow(character *ic, byte row, char firstChar);
void inspectCharacter(byte idx);

item *whichItem(character *ic, byte *inventorySlot, byte *equipmentSlot);
void useItem(character *ic);

#define nameOfInventoryItemWithID(I) nameOfInventoryItem(inventoryItemForID(I))
#define getWeapon(aCharacter)   inventoryItemForID(aCharacter->weapon)
#define getArmor(aCharacter)    inventoryItemForID(aCharacter->armor)
#define getShield(aCharacter)   inventoryItemForID(aCharacter->shield)

#endif