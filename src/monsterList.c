#include "types.h"
#include <stddef.h>

// monsters

monsterDef gMonsters[]= {

    // -------------------- minor monsters ---------------------- */

    {
        0x00, 1, 10, mt_humanoid,           // ID, default level, spriteID
        "Noob", NULL,         // name, pluralname
        9, 6, 6, 0, 1,        // armorClass, hit dice, hit points, mag points, numAttacks
        0, 0,                 // courageMod, hitMod
        at_fists | at_weapon, // attack types
        1                     // xp value
    },

    {
        0x01, 1, 11, mt_humanoid,           // ID, default level, spriteID
        "Kobold", NULL,       // name, pluralname
        10, 6, 6, 0, 1,        // armorClass, hit dice, hit points, numAttacks
        0, 0,                 // courageMod, hitMod
        at_fists | at_weapon, // attack types
        10                    // xp value
    },

        {
        0x02, 1, 12, mt_humanoid,           // ID, default level, spriteID
        "Orc", NULL,          // name, pluralname
        9, 6, 6, 0, 1,        // armorClass, hit dice, hit points, numAttacks
        0, 0,                 // courageMod, hitMod
        at_fists | at_weapon, // attack types
        10                    // xp value
    },

    {
        0x03, 1, 13, mt_humanoid,           // ID, default level, spriteID
        "Kobold General", NULL,       // name, pluralname
        10, 6, 6, 0, 2,        // armorClass, hit dice, hit points, numAttacks
        0, 0,                 // courageMod, hitMod
        at_fists | at_weapon, // attack types
        15                    // xp value
    },

    // ---------------------- bosses ---------------------------
    // (all monsters above 0xa0 are unique)

    {
        0xa0, 10, 0xa0, mt_magical | mt_unique,                      // ID, default level, spriteID
        "Obenzna, Dragon of Mistrust", NULL, // name, pluralname
        3, 12, 12, 14, 4, // armorClass, hit dice, hit points, magPoints, numAttacks
        2, 3,         // courageMod, hitMod
        at_claws | at_breath | at_ice, // attack types
        900                            // xp value
    },

    {
        0xff, 102, 0xff, mt_magical | mt_unique,                           // ID, default level, spriteID
        "Gerulda, the Great Ego Dragon", NULL,     // name, pluralname
        0, 12, 12, 20, 4,                              // armorClass, hit dice, hit points, numAttacks
        10, 5,                                     // courageMod, hitMod
        at_claws | at_breath | at_ice | at_spell,  // attack types
        10000                                      // xp value
    },

};