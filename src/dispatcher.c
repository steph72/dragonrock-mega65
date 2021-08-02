#include "dispatcher.h"
#include <cbm.h>
//#include <conio.h>
#include <device.h>
#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "city.h"
#include "dungeon.h"
#include "encounter.h"
#include "guild.h"
#include "memory.h"
#include "utils.h"
#include <c64.h>

extern unsigned int _OVERLAY1_LOAD__[], _OVERLAY1_SIZE__[];
extern unsigned int _OVERLAY2_LOAD__[], _OVERLAY2_SIZE__[];
extern unsigned int _OVERLAY3_LOAD__[], _OVERLAY3_SIZE__[];

gameModeT gCurrentGameMode;
gameModeT gNextGameMode;
gameModeT lastGameMode;

byte gCurrentDungeonIndex;
byte gLoadedDungeonIndex;
byte gStartXPos;
byte gStartYPos;
byte gCurrentCityIndex;

encResult gEncounterResult;

void prepareForGameMode(gameModeT newGameMode) { gNextGameMode= newGameMode; }

void popLastGameMode(void) { gNextGameMode= lastGameMode; }

void loadModules(void) {
    loadExt("dungeon", ATTIC_DUNGEON_CODE, true);
    loadExt("city", ATTIC_CITY_CODE, true);
    loadExt("encounter", ATTIC_ENCOUNTER_CODE, true);
}

void commitNewGameMode(void) {

    if (gNextGameMode == gCurrentGameMode) {
        return;
    }

    lastGameMode= gCurrentGameMode;
    gCurrentGameMode= gNextGameMode;

    switch (gNextGameMode) {

    case gm_dungeon:
    case gm_outdoor:
        if (lastGameMode != gm_dungeon && lastGameMode != gm_outdoor) {
            cg_bordercolor(COLOR_BLUE);
            lcopy(ATTIC_DUNGEON_CODE, (long)_OVERLAY1_LOAD__,
                  (unsigned int)_OVERLAY1_SIZE__);
        }
        break;

    case gm_city:
        // bordercolor(COLOR_GREEN);
        lcopy(ATTIC_CITY_CODE, (long)_OVERLAY2_LOAD__,
              (unsigned int)_OVERLAY2_SIZE__);
        break;

    case gm_encounter:
        cg_bordercolor(COLOR_RED);
        lcopy(ATTIC_ENCOUNTER_CODE, (long)_OVERLAY3_LOAD__,
              (unsigned int)_OVERLAY3_SIZE__);
        break;

    case gm_init:
        cg_fatal("gm init");
        break;

    default:
        break;
    }
}

void enterCurrentGameMode() {

    switch (gCurrentGameMode) {

    case gm_city:
        enterCityMode();
        break;

    case gm_dungeon:
    case gm_outdoor:
        // make sure that dungeon/map gets re-initialized
        // if not coming from an encounter...
        enterDungeonMode(lastGameMode != gm_encounter);
        break;

    case gm_encounter:
        gEncounterResult= doEncounter();
        clearMonsters();
        if (gEncounterResult != encDead) {
            popLastGameMode();
        } else {
            prepareForGameMode(gm_city);
        }
        break;

    default:
        break;
    }
}

void mainDispatchLoop(void) {
    while (gNextGameMode != gm_end) {
        // printf("COMMIT GAME MODE %d",gNextGameMode);
        // cgetc();
        commitNewGameMode();
        enterCurrentGameMode();
    }
}
