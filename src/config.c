#include "config.h"

char *gRaces[NUM_RACES] = {"Human", "Dwarf", "Elf", "Half-Elf", "Gnome"};
char *gRacesS[NUM_RACES] = {"H", "D", "E", "HE", "G"};

char *gAttributes[] = {"Strength", "Intelligence", "Wisdom",
                       "Dexterity", "Constitution", "Charisma"};
                       
char *gAttributesS[] = {"STR", "INT", "WIS", "DEX", "CON", "CHR"};

int gRaceModifiers[NUM_RACES][6] = {
    {0, 0, 0, 0, 0, 0},   // human
    {2, 0, 0, -1, 0, 0},  // dwarf
    {-1, 2, 0, 0, -1, 1}, // elf
    {0, 1, 0, 0, 0, 0},   // half-elf
    {-1, 1, 1, 2, -1, 0}  // gnome
};

char *gClasses[NUM_CLASSES] = {"Fighter", "Ranger", "Priest", "Wizard", "Thief"};
char *gClassesS[NUM_CLASSES] = {"FIG", "RAN", "PRI", "WIZ", "THF"};

char *gCities[NUM_CITIES] = {"Foxhome", "King's Winter", "Dohlem", "Tia's Grove",
                             "Modder", "Foodim"};