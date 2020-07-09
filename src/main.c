/*
 * Copyright (C) 2019  Stephan Kleinert
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have recxeived a copy of the GNU Lesser General Public
 * License along with main.c; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include <6502.h>
#include <conio.h>
#include <em.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cbm.h>
#include <device.h>
#include <time.h>

#include "charset.h"
#include "irq.h"

#include "globals.h"

#include "congui.h"
#include "debug.h"
#include "dungeon.h"
#include "guild.h"

#include "encounter.h"
#include "monster.h"
#include "spell.h"

#include "dispatcher.h"

#include "c65.h"

#ifndef DRE_VERSION
#define DRE_VERSION "0.1a"
#endif

#ifndef DRE_DATE
#define DRE_DATE "12/29/2019"
#endif

#ifndef DRE_BUILDNUM
#define DRE_BUILDNUM "-666"
#endif

char *drbuf;

byte hasLoadedGame;

void initEngine(void);
void runCityMenu(void);
void doGuild(void);
void loadSaved(void);
void installCharset(void);

const char *prompt=
    "DREngine/C65 V" DRE_VERSION " build " DRE_BUILDNUM "\n" DRE_DATE "\n\n";

void testMachine(void) {
    if (!testVIC3()) {
        bordercolor(0);
        bgcolor(0);
        textcolor(COLOR_LIGHTRED);
        clrscr();
        cputs("We're awfully sorry, but DragonRock\r\n"
              "needs a C65 or MEGA65 computer to run.\r\n");
        exit(0);
    }
}

void initEngine(void) {
    unsigned int rseed;

    testMachine();
    drbuf= malloc(0xff);
    cg_init();
    puts(prompt);
    sleep(1);
    rseed= 42;
    srand(rseed);
    /*
    // TODO
    if (cbm_load("charset", getcurrentdevice(), (void *)0xf800) == 0) {
        puts("Failed loading charset.");
        exit(0);
    }
    installIRQ();
    */
    hasLoadedGame= loadParty();
    /* enableCustomCharset(); */
    gLoadedDungeonIndex= 255;
    gPartyExperience= 1000;
    gPartyGold= 1000;
    gCurrentGameMode= gm_init;
}

void debugDungeon(void) {
    gCurrentDungeonIndex= 0;
    gCurrentGameMode= gm_init;
    prepareForGameMode(gm_dungeon);
    mainDispatchLoop();
}

void debugEncounter(void) {

    byte i;

    for (i= 0; i < 5; i++) {
        setHasSpell(party[0], i);
        setHasSpell(party[1], i + 5);
        party[i]->gold= 100;
    }

    clearMonsters();
    gCurrentDungeonIndex= 0;
    addNewMonster(0, 1, 6, 0);
    addNewMonster(1, 1, 3, 1);
    addNewMonster(2, 1, 1, 2);
    prepareForGameMode(gm_dungeon);
    gEncounterResult= doPreEncounter(); // try pre-encounter first
    if (gEncounterResult == encFight) {
        gCurrentGameMode= gm_dungeon; // simulate coming from dungeon
        prepareForGameMode(gm_encounter);
        /*
             a real fight? -->
             quit dungeon and let dispatcher handle loading the
             rest of the encounter module
        */
    }
    mainDispatchLoop();
}

int main() {
    static char choice;
    byte i;

    initEngine();
    clrscr();
    cg_borders();

    cputsxy(2, 11, "1 - load saved game");
    cputsxy(2, 13, "2 - start in ");
    cputs(gCities[0]);

    gCurrentCityIndex= 0;
    prepareForGameMode(gm_city);

    cputsxy(2, 20, "Select option: ");

    cursor(1);
    do {
        choice= cgetc();
    } while (strchr("12de", choice) == NULL);
    cursor(0);

    if (choice == 'd') {
        debugDungeon();
    }

    if (choice == 'e') {
        debugEncounter();
    }

    if (choice == '1' && hasLoadedGame) {
        // determine last city from saved party
        gCurrentCityIndex= party[0]->city;
    } else {
        // remove saved party if not loading saved game
        for (choice= 0; choice < PARTYSIZE; party[choice++]= 0)
            ;
    }

    mainDispatchLoop();

    return 0;
}
