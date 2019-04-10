#include "config.h"
#include "types.h"

char *gRaces[NUM_RACES]= {"Human", "Dwarf", "Elf", "Half-Elf", "Gnome"};
char *gRacesS[NUM_RACES]= {"H", "D", "E", "HE", "G"};

char *gAttributes[]= {"Strength",  "Intelligence", "Wisdom",
                      "Dexterity", "Constitution", "Charisma"};

char *gAttributesS[]= {"STR", "INT", "WIS", "DEX", "CON", "CHR"};

char *gStateDesc[]= {"deleted", "dead", "ok"};

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

    {0x00, "--", it_special, 0, 0, 0},

    // --- weapons ---

    {0x01, "Club", it_weapon, 1, 3, 1},
    {0x02, "Staff", it_weapon, 1, 3, 1},
    {0x03, "Dagger", it_weapon, 1, 4, 10},
    {0x04, "Short sword", it_weapon, 1, 6, 20},

    // --- shields ---

    {0x40, "Small shield", it_shield, 0, 2, 10},
    {0x41, "Medium shield", it_shield, 1, 3, 20},
    {0x42, "Large schield", it_shield, 2, 4, 30},

    // --- armor ---

    {0x80, "Robe", it_armor, 0, 1, 1},

    // --- special ---

    {0xff, "White Orb", it_special, 0, 0, 65535ul}

};