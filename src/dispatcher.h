
#ifndef dispatcher_d
#define dispatcher_d

#include "types.h"

typedef enum _gameMode {
    gm_init,
    gm_city,
    gm_outdoor,
    gm_dungeon,
    gm_encounter,
    gm_end
} gameModeT;

extern gameModeT gCurrentGameMode;
extern byte gCurrentDungeonIndex;
extern byte gLoadedDungeonIndex;
extern byte gCurrentCityIndex;

extern encResult gEncounterResult;

void mainDispatchLoop(void);
void prepareForGameMode(gameModeT newGameMode);
void popLastGameMode(void);

void commitNewGameMode(void); // TODO: make this private after debugging is done

#endif