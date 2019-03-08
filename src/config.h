#ifndef __dwConfig
#define __dwConfig

// these may change...

#define GUILDSIZE 40
#define PARTYSIZE 6

#define NUM_CITIES 6
#define NUM_CLASSES 5
#define NUM_RACES 5

#define NUM_ATTRS 6 // ...but change this only if you know what you're doing

#define DR_GUILD_ID "**bkckkksk7t_dr**"

extern char *gRaces[NUM_RACES];
extern char *gRacesS[NUM_RACES];
extern char *gClasses[NUM_CLASSES];
extern char *gClassesS[NUM_CLASSES];
extern char *gAttributes[NUM_ATTRS];
extern char *gAttributesS[NUM_ATTRS];

extern char *gCities[NUM_CITIES];

extern int gRaceModifiers[NUM_RACES][6];

#endif