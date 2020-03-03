#include "globals.h"

// check whether spell is available for aChar
byte hasSpell(character *aChar, byte spellID);

// make spell available for aChar
void setHasSpell(character *aChar, byte spellID);

// return spell for given spell ID
#define spellForSpellID(spellID) gSpells[spellID]

// return name for given spell
char *nameOfSpell(spell *aSpell);
char *nameOfSpellWithID(byte spellID);

// cast spell
byte castSpell(character *aChar);

// determine whether spell takes character destination
byte spellNeedsCharacterDestination(byte spellID);

// determine whether spell takes row destination
byte spellNeedsRowDestination(byte spellID);
