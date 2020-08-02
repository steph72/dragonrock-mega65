#include "types.h"
#include "dispatcher.h"

// these may change...

#define GUILDSIZE 36
#define PARTYSIZE 6

#define NUM_CITIES 6
#define NUM_CLASSES 6
#define NUM_RACES 5

#define DR_GUILD_ID "**bkckkksk7t_dr**"

extern char *gRaces[NUM_RACES];
extern char *gRacesS[NUM_RACES];
extern char *gClasses[NUM_CLASSES];
extern char *gClassesS[NUM_CLASSES];
extern char *gAttributes[NUM_ATTRS];
extern char *gAttributesS[NUM_ATTRS];

extern char *gCities[NUM_CITIES];
extern char *gStateDesc[];

extern cityCoordsT gCityCoords[NUM_CITIES];

extern item gItems[];
extern spell gSpells[];
extern monsterDef gMonsters[];

extern int gRaceModifiers[NUM_RACES][6];
