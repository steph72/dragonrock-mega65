#include "globals.h"
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

char *gClasses[NUM_CLASSES]= {"Fighter", "Ranger", "Priest",
                              "Wizard",  "Thief",  "Monk"};
char *gClassesS[NUM_CLASSES]= {"FG", "RG", "PR", "WZ", "TH"};

char *gCities[NUM_CITIES]= {"Foxhome",     "King's Winter", "Dohlem",
                            "Tia's Grove", "Modder",        "Foodim"};

cityCoordsT gCityCoords[NUM_CITIES]= {{33, 21, 15}, {0, 0, 0}, {0, 0, 0},
                                      {0, 0, 0},    {0, 0, 0}, {0, 0, 0}};

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
