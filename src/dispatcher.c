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

extern void _OVERLAY1_LOAD__[], _OVERLAY1_SIZE__[];
extern void _OVERLAY2_LOAD__[], _OVERLAY2_SIZE__[];
extern void _OVERLAY3_LOAD__[], _OVERLAY3_SIZE__[];

gameModeT gCurrentGameMode;
gameModeT gNextGameMode;
gameModeT lastGameMode;

byte gCurrentDungeonIndex;
byte gLoadedDungeonIndex;
byte gCurrentCityIndex;

encResult gEncounterResult;

unsigned char loadfile(char *name, void *addr, void *size);

void prepareForGameMode(gameModeT newGameMode) { gNextGameMode= newGameMode; }

void popLastGameMode(void) { gNextGameMode= lastGameMode; }

void commitNewGameMode(void) {

    if (gNextGameMode == gCurrentGameMode) {
        return;
    }

    lastGameMode= gCurrentGameMode;

    gCurrentGameMode= gNextGameMode;

    switch (gNextGameMode) {

    case gm_dungeon:
        loadfile("dungeon", _OVERLAY1_LOAD__, _OVERLAY1_SIZE__);
        break;

    case gm_city:
        loadfile("city", _OVERLAY2_LOAD__, _OVERLAY2_SIZE__);
        break;

    case gm_encounter:
        clrscr();
        cputs("An encounter...");
        loadfile("encounter", _OVERLAY3_LOAD__, _OVERLAY3_SIZE__);
        break;

    default:
        break;
    }
}

void enterCurrentGameMode() {

    switch (gCurrentGameMode) {

    case gm_city:
        initGuild();
        runCityMenu();
        break;

    case gm_dungeon:
        enterDungeonMode();
        break;

    case gm_encounter:
        gEncounterResult= doEncounter();
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
        commitNewGameMode();
        enterCurrentGameMode();
    }
}

unsigned char loadfile(char *name, void *addr, void *size) {
#ifdef DEBUG
    byte x, y;
#endif
    /* Avoid compiler warnings about unused parameters. */
    (void)addr;
    (void)size;
#ifdef DEBUG
    x= wherex();
    y= wherey();
    gotoxy(0, 24);
    printf("l %s $%x @ $%x ", name, size, addr);
    printf("$%x rem", 0x2000 - (int)size);
    gotoxy(x, y);
#endif
    if (cbm_load(name, getcurrentdevice(), NULL) == 0) {
        cputs("Loading overlay file failed");
        exit(0);
    }
    return 1;
}
