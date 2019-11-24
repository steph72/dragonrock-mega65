#include <stddef.h>
#include "config.h"
#include "types.h"

char *gRaces[NUM_RACES]= {"Human", "Dwarf", "Elf", "Half-Elf", "Gnome"};
char *gRacesS[NUM_RACES]= {"H", "D", "E", "HE", "G"};

char *gAttributes[]= {"Strength",  "Intelligence", "Wisdom",
                      "Dexterity", "Constitution", "Charisma"};

char *gAttributesS[]= {"STR", "INT", "WIS", "DEX", "CON", "CHR"};

char *gStateDesc[]= {"deleted", "down", "asleep", "dead", "ok"};

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



// equipment

item gItems[]= {

    // --- nothing ---

    {0x00, "--", it_special, 0, 0, 0, 0},

    // --- weapons ---

    {0x01, "Club", it_weapon, 1, 3, 0, 1},
    {0x02, "Staff", it_weapon, 1, 3, 0, 1},
    {0x03, "Dagger", it_weapon, 1, 4, 0, 10},
    {0x04, "Short sword", it_weapon, 1, 6, 0, 20},

    // --- shields ---

    {0x40, "Small shield", it_shield, 0, 2, 0, 10},
    {0x41, "Medium shield", it_shield, 1, 3, 0, 20},
    {0x42, "Large schield", it_shield, 2, 4, 0, 30},

    // --- armor ---

    {0x80, "Robe", it_armor, 0, 1, 0, 1},

    // --- scrolls and books ---

    {0xa0, "Scroll 1", it_scroll, 1, 0, 0, 1},
    {0xa1, "Scroll 2", it_scroll, 2, 0, 0, 1},
    {0xa2, "Scroll 3", it_scroll, 3, 0, 0, 1},
    {0xa3, "Scroll 4", it_scroll, 4, 0, 0, 1},
    {0xa4, "Scroll 5", it_scroll, 5, 0, 0, 1},
    {0xa5, "Scroll 6", it_scroll, 6, 0, 0, 1},
    {0xa6, "Scroll 7", it_scroll, 7, 0, 0, 1},
    {0xa7, "Scroll 8", it_scroll, 8, 0, 0, 1},
    {0xa8, "Scroll 9", it_scroll, 9, 0, 0, 1},
    {0xa9, "Scroll 10", it_scroll, 10, 0, 0, 1},

    // --- special ---
    {0xf0, "Rusty Key", it_special, 0, 0, 0, 65535u},
    {0xff, "White Orb", it_special, 0, 0, 0, 65535u}

};