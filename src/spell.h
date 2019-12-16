#include "types.h"
#include "character.h"
#include "config.h"

// check whether spell is available for aChar
byte hasSpell(character *aChar, byte spellID);

// make spell available for aChar
void setHasSpell(character *aChar, byte spellID);

// return spell for given spell ID
spell spellForSpellID(byte spellID);

// return name for spell with given spell ID
char *nameOfSpellWithSpellID(byte spellID);

// return name for given spell
char *nameOfSpell(spell *aSpell);

// cast spell
byte castSpell(character *aChar, spell *aSpell);
