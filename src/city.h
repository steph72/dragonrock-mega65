
#ifndef _city
#define _city

#include "types.h"

#define getNameForCityID(A) getNameForCityDef(getCityDef(A))
#define getInnNameForCityID(A) getInnNameForCityDef(getCityDef(A))
#define getArmorerNameForCityID(A) getArmorerNameForCityDef(getCityDef(A))


void enterCityMode(void);
cityDef *getCityDef(byte cityID);
char *getNameForCityDef(cityDef *def);
char *getInnNameForCityDef(cityDef *def);
char *getArmorerNameForCityDef(cityDef *def);

#endif
