#include "config.h"
#include "types.h"
#include <stddef.h>

char *gRaces[NUM_RACES]= {"Human", "Dwarf", "Elf", "Half-Elf", "Gnome"};
char *gRacesS[NUM_RACES]= {"H", "D", "E", "HE", "G"};

char *gAttributes[]= {"Strength",  "Intelligence", "Wisdom",
                      "Dexterity", "Constitution", "Charisma"};

char *gAttributesS[]= {"STR", "INT", "WIS", "DEX", "CON", "CHR"};

char *gStateDesc[]= {"deleted", "down", "asleep", "dead", "ok", "fled"};

int gRaceModifiers[NUM_RACES][6]= {
    {0, 0, 0, 0, 0, 0},   // human
    {2, 0, 0, -1, 0, 0},  // dwarf
    {-1, 2, 0, 0, -1, 1}, // elf
    {0, 1, 0, 0, 0, 0},   // half-elf
    {-1, 1, 1, 2, -1, 0}  // gnome
};

char *gClasses[NUM_CLASSES]= {"Fighter", "Ranger", "Priest", "Wizard", "Thief"};
char *gClassesS[NUM_CLASSES]= {"FG", "RG", "PR", "WZ", "TH"};

char *gCities[NUM_CITIES]= {"Foxhome",     "King's Winter", "Dohlem",
                            "Tia's Grove", "Modder",        "Foodim"};

// spells
// name, spellLevel, minLevel, mpNeeded, minVal, maxVal

spell gSpells[]= {

    /*  0 */ {"--", 0, 0, 0, 0, 0}, // nothing

    /*  1 */ {"Heal", 1, 1, 2, 1, 4},   // heal 1
    /*  2 */ {"Heal", 2, 3, 4, 2, 8},   // heal 2
    /*  3 */ {"Heal", 3, 6, 8, 4, 10},  // heal 3
    /*  4 */ {"Heal", 4, 8, 16, 6, 15}, // heal 4

    /*  5 */ {"Fireflash", 1, 1, 2, 1, 4},   // fireflash 1
    /*  6 */ {"Fireflash", 2, 3, 4, 2, 8},   // fireflash 2
    /*  7 */ {"Fireflash", 3, 6, 8, 4, 10},  // fireflash 3
    /*  8 */ {"Fireflash", 4, 8, 16, 6, 15}, // fireflash 4

};

// equipment

item gItems[]= {

    // --- nothing ---

    {0x00, "--", it_special, 0, 0, 0, 0},

    // --- weapons ---

    {0x01, "Club", it_weapon, 1, 3, 0, 1},
    {0x02, "Staff", it_weapon, 1, 3, 0, 5},
    {0x03, "Dagger", it_weapon, 1, 4, 0, 10},
    {0x04, "Short sword", it_weapon, 1, 6, 0, 20},

    // --- bows & slings ---

    {0x10, "Sling", it_missile, 1, 4, 0, 1},
    {0x20, "Short bow", it_missile, 3, 6, 0, 10},
    {0x21, "Short bow", it_missile, 5, 6, 1, 100},
    {0x22, "Short bow", it_missile, 8, 6, 2, 1000},
    {0x23, "Short bow", it_missile, 12, 6, 3, 5000},
    {0x24, "Short bow", it_missile, 12, 6, 5, 20000},

    // --- shields ---

    {0x40, "Small shield", it_shield, 0, 2, 0, 10},
    {0x41, "Medium shield", it_shield, 1, 3, 0, 20},
    {0x42, "Large shield", it_shield, 2, 4, 0, 30},

    // --- armor ---

    {0x80, "Robe", it_armor, 0, 1, 0, 2},

    // --- scrolls and books ---

    {0xa0, "Scroll", it_scroll, 1, 0, 0, 1},
    {0xa1, "Scroll", it_scroll, 2, 0, 0, 1},
    {0xa2, "Scroll", it_scroll, 3, 0, 0, 1},
    {0xa3, "Scroll", it_scroll, 4, 0, 0, 1},
    {0xa4, "Scroll", it_scroll, 5, 0, 0, 1},
    {0xa5, "Scroll", it_scroll, 6, 0, 0, 1},
    {0xa6, "Scroll", it_scroll, 7, 0, 0, 1},
    {0xa7, "Scroll", it_scroll, 8, 0, 0, 1},
    {0xa8, "Scroll", it_scroll, 9, 0, 0, 1},
    {0xa9, "Scroll", it_scroll, 10, 0, 0, 1},

    // --- special ---
    {0xf0, "Rusty Key", it_special, 0, 0, 0, 65535u},
    {0xff, "White Orb", it_special, 0, 0, 0, 65535u}

};