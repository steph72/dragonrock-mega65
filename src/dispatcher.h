/*
 * =====================
 * DragonRock dispatcher
 * =====================
 *
 * Loading and switching between game module overlays.
 *
 */

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

/**
 * mainDispatchLoop:
 * the main dispatch loop is responsible for loading and
 * initializing the various game modules. once a module
 * has finished, the dispatcher loads the next module which
 * was configured via the prepareForGameMode() function
 */
void mainDispatchLoop(void);

/**
 * prepareNewGameMode:
 * signals the dispatcher which new module to load once the current module has ended
 */
void prepareForGameMode(gameModeT newGameMode);

/**
 * popLastGameMode:
 * signals the dispatcher to load the last used module 
 * before the current module became active. used by the
 * encounter game mode to get back to whichever game
 * mode started the encounter
 */
void popLastGameMode(void);

/**
 * commitNewGameMode loads the prepared game module,
 * saves the last active game module and does the
 * necessary initializations
 */
void commitNewGameMode(void); // TODO: make this private after debugging is done

#endif