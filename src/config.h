#include "types.h"
#include "dispatcher.h"

// these may change...

#define GUILDSIZE 24
#define PARTYSIZE 6

#define NUM_CITIES 7
#define NUM_CLASSES 6
#define NUM_RACES 5

extern char *gRaces[NUM_RACES];
extern char *gRacesS[NUM_RACES];
extern char *gClasses[NUM_CLASSES];
extern char *gClassesS[NUM_CLASSES];
extern char *gAttributes[NUM_ATTRS];
extern char *gAttributesS[NUM_ATTRS];

extern char *gStateDesc[];
extern spell gSpells[];

extern signed char gRaceModifiers[NUM_RACES][6];
