#include "spell.h"
#include <stdio.h>

static byte spellMapByteIdx;
static byte spellMapBitIdx;

byte hasSpell(character *aChar, byte spellID) {
    spellMapByteIdx= spellID / 8;
    spellMapBitIdx= spellID % 8;
    return aChar->spellMap[spellMapByteIdx] & (1 << (spellMapBitIdx));
}

void setHasSpell(character *aChar, byte spellID) {
    spellMapByteIdx= spellID / 8;
    spellMapBitIdx= spellID % 8;
    aChar->spellMap[spellMapByteIdx]|= (1 << spellMapBitIdx);
}

char *nameOfSpell(spell *aSpell) {
    if (aSpell->spellLevel == 0) {
        return aSpell->name;
    }
    sprintf(drbuf, "%s %d", aSpell->name, aSpell->spellLevel);
    return drbuf;
}

char *nameOfSpellWithID(byte spellID) { return nameOfSpell(&gSpells[spellID]); }

byte spellNeedsCharacterDestination(byte spellID) {
    return (spellID >= 1 && spellID <= 4);  // healing spells
}
