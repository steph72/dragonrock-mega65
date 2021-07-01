#include "dispatcher.h"
#include <cbm.h>
#include <conio.h>
#include <device.h>
#include <stdio.h>
#include <stdlib.h>

#include "city.h"
#include "dungeon.h"
#include "encounter.h"
#include "guild.h"
#include "memory.h"
#include <c64.h>

extern unsigned int _OVERLAY1_LOAD__[], _OVERLAY1_SIZE__[];
extern unsigned int _OVERLAY2_LOAD__[], _OVERLAY2_SIZE__[];
extern unsigned int _OVERLAY3_LOAD__[], _OVERLAY3_SIZE__[];

#define ATTIC_DUNGEON   0x8010000
#define ATTIC_CITY      0x8014000
#define ATTIC_ENCOUNTER 0x8018000

gameModeT gCurrentGameMode;
gameModeT gNextGameMode;
gameModeT lastGameMode;

byte gCurrentDungeonIndex;
byte gLoadedDungeonIndex;
byte gStartXPos;
byte gStartYPos;
byte gCurrentCityIndex;

encResult gEncounterResult;

unsigned char loadfile(char *name, void *addr, void *size);

void prepareForGameMode(gameModeT newGameMode) { gNextGameMode= newGameMode; }

void popLastGameMode(void) { gNextGameMode= lastGameMode; }

void loadModule(char *name){
    printf("lmod %s\n",name);
    loadfile(name,(void*)0x9000,(void*)0x4000);
}

void loadModules(void) {
    loadModule("dungeon");
    lcopy(0x9000,ATTIC_DUNGEON,0x4000);
    loadModule("city");
    lcopy(0x9000,ATTIC_CITY,0x4000);
    loadModule("encounter");
    lcopy(0x9000,ATTIC_ENCOUNTER,0x4000);
}

void commitNewGameMode(void) {

    if (gNextGameMode == gCurrentGameMode) {
        return;
    }

    lastGameMode= gCurrentGameMode;
    gCurrentGameMode= gNextGameMode;

    cg_clrscr();

    switch (gNextGameMode) {

    case gm_dungeon:
    case gm_outdoor:
        if (lastGameMode != gm_dungeon && lastGameMode != gm_outdoor) {
            bordercolor(COLOR_BLUE);
            lcopy(ATTIC_DUNGEON,(long)_OVERLAY1_LOAD__,(unsigned int)_OVERLAY1_SIZE__);
        }
        break;

    case gm_city:
        // bordercolor(COLOR_GREEN);
        lcopy(ATTIC_CITY,(long)_OVERLAY2_LOAD__,(unsigned int)_OVERLAY2_SIZE__);
        break;

    case gm_encounter:
        bordercolor(COLOR_RED);
        lcopy(ATTIC_ENCOUNTER,(long)_OVERLAY3_LOAD__,(unsigned int)_OVERLAY3_SIZE__);
        break;

    case gm_init:
        puts("??new gamemode is init");
        while(1);
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
        enterDungeonMode(lastGameMode!=gm_encounter);
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

unsigned char loadfile(char *name, void *addr, void *size) {
    /* Avoid compiler warnings about unused parameters. */
    (void)addr;
    (void)size;
    
    if (cbm_load(name, getcurrentdevice(), NULL) == 0) {
        cputs("Loading overlay file failed");
        while(1);
    }
    return 1;
}
