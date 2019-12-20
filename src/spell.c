#include "spell.h"
#include "utils.h"
#include <conio.h>
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

byte isHealingSpell(byte spellID) { return (spellID >= 1 && spellID <= 4); }

byte spellNeedsCharacterDestination(byte spellID) {
    return isHealingSpell(spellID);
}

void announceSpell(character *aChar) {
    cprintf("%s casts %s\r\n", aChar->name,
            nameOfSpellWithID(aChar->encSpell));
}

byte castHealingSpell(character *srcCharacter) {
    byte healVal;
    character *destCharacter;
    spell *aSpell;

    destCharacter= party[srcCharacter->encDestination-1];
    aSpell= &gSpells[srcCharacter->encSpell];

    announceSpell(srcCharacter);

    healVal= dmrand(aSpell->minDmg, aSpell->maxDmg);
    if (destCharacter->aHP + healVal > destCharacter->aMaxHP) {
        healVal-= (destCharacter->aHP + healVal - destCharacter->aMaxHP);
    }
    destCharacter->aHP+= healVal;
    cprintf("%s is healed for %d points", destCharacter->name, healVal);
    return true;
}

byte castSpell(character *aChar) {

    byte castSuccessful = false;

    if (aChar->encSpell == 0) {
        return false;
    }
    switch (aChar->encSpell) {
    case 1:
    case 2:
    case 3:
    case 4:
        castSuccessful = castHealingSpell(aChar);
        break;

    default:
        break;
    }

    if (castSuccessful) {
        aChar->aMP -= gSpells[aChar->encSpell].mpNeeded;
    }


    return castSuccessful;
}
