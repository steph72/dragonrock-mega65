#include "spell.h"
#include "globals.h"
#include "utils.h"
#include "congui.h"
//#include <conio.h>
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

byte isHealingSpell(byte spellID) {
    return (spellID >= 1 && spellID <= 4); // healing spell
}

byte spellNeedsRowDestination(byte spellID) {
    return (spellID >= 5 && spellID <= 8); // fireflash spell
}

byte spellNeedsCharacterDestination(byte spellID) {
    return isHealingSpell(spellID);
}

void announceSpell(character *aChar) {
    cg_printf("%s casts %s\n", aChar->name, nameOfSpellWithID(aChar->encSpell));
}

// clang-format off
#pragma code-name(push, "OVERLAY3");
// clang-format on

byte castFireflashSpell(character *aCharacter) {
    byte dmgVal;
    spell *aSpell;

    aSpell= &gSpells[aCharacter->encSpell];
    announceSpell(aCharacter);
    dmgVal= dmrand(aSpell->minDmg, aSpell->maxDmg);
    return true;
}

// clang-format off
#pragma code-name(pop);
// clang-format on

byte castHealingSpell(character *srcCharacter) {
    byte healVal;
    character *destCharacter;
    spell *aSpell;

    destCharacter= party[srcCharacter->encDestination - 1];
    aSpell= &gSpells[srcCharacter->encSpell];

    announceSpell(srcCharacter);

    healVal= dmrand(aSpell->minDmg, aSpell->maxDmg);
    destCharacter->aHP+= healVal;

    if (destCharacter->aMaxHP > destCharacter->aMaxHP) {
        destCharacter->aHP= destCharacter->aMaxHP;
    }
    destCharacter->aHP+= healVal;
    cg_printf("%s is healed.", destCharacter->name);
    if (destCharacter->status == down && destCharacter->aHP > 0) {
        cg_printf("\n%s gets up again!", destCharacter->name);
        destCharacter->status= awake;
    }
    return true;
}

byte castSpell(character *aChar) {

    byte castSuccessful= false;

    if (aChar->encSpell == 0) {
        return false;
    }
    switch (aChar->encSpell) {
    case 1:
    case 2:
    case 3:
    case 4:
        castSuccessful= castHealingSpell(aChar);
        break;

    case 5:
    case 6:
    case 7:
    case 8:
    if (gCurrentGameMode==gm_encounter) {
        castSuccessful= castFireflashSpell(aChar);
    } else {
        puts("only in encounter!");
        cg_getkey();
    }
        break;

    default:
        break;
    }

    if (castSuccessful) {
        aChar->aMP-= gSpells[aChar->encSpell].mpNeeded;
    }

    return castSuccessful;
}
