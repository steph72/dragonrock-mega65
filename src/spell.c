#include "spell.h"

byte hasSpell(character *aChar, byte spellID) {

    byte byteIdx;
    byte bitIdx;
    byte hasSpell;

    byteIdx = spellID/8;
    bitIdx = spellID%8;
    hasSpell = aChar->spellMap[byteIdx] & (1<<bitIdx);

    return hasSpell;

}

void setHasSpell(character *aChar, byte spellID) {

    byte byteIdx;
    byte bitIdx;

    byteIdx = spellID/8;
    bitIdx = spellID%8;

    aChar->spellMap[byteIdx] |= (1<<bitIdx);

}

