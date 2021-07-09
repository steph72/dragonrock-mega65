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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cbm.h>
#include <device.h>
#include <time.h>

#include "globals.h"

#include "congui.h"
#include "debug.h"
#include "dungeon.h"
#include "guild.h"

#include "encounter.h"
#include "monster.h"
#include "spell.h"

#include "dispatcher.h"
#include "memory.h"

#include "menu.h"
#include "utils.h"

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
byte devmode;

void initEngine(void);
void runCityMenu(void);
void doGuild(void);
void loadSaved(void);

const char *prompt=
    "DREngine/m65 v" DRE_VERSION " build " DRE_BUILDNUM "\n" DRE_DATE "\n\n";

void loadResources(void) {
    unsigned int readBytes;
    // printf("load items; ");
    readBytes= loadExt("items", ITEM_BASE);
    // printf("$%x bytes read\n", readBytes);
    // printf("load monsters; ");
    readBytes= loadExt("monsters", MONSTERS_BASE);
    // printf("$%x bytes read\n", readBytes);
}

void initEngine(void) {
    mega65_io_enable();

    drbuf= (char *)malloc(DRBUFSIZE);

    srand((unsigned int)DRE_BUILDNUM);
    puts("\n");       // cancel leftover quote mode from wrapper or whatever
    cbm_k_bsout(14);  // lowercase
    cbm_k_bsout(147); // clr
    puts(prompt);
    lcopy(0x5f000, (long)drbuf, 4);
    if (drbuf[0] == 0x53 && drbuf[1] == 0x4b) {
        devmode= true;
    } else if (drbuf[0] != 0x23 || drbuf[1] != 0x45) {
        cg_fatal("Please BOOT the dragon rock disc to\n"
                 "correctly initialize the game.");
    }

    lpoke (0x5f000,0);
    lpoke (0x5f001,0);
 
    loadModules();
    // puts("init monster rows");
    initMonsterRows();
    loadResources();
    // puts("init party");
    hasLoadedGame= loadParty();
    gLoadedDungeonIndex= 255;
    gCurrentGameMode= gm_init;
    cg_init();

    // after graphics initialization, it's safe to free drbuf and using
    // the old text screen instead, thus saving a few bytes...
    // 798e-77c0

    free(drbuf);
    drbuf= (char *)0x400;

    // cg_test();

    /*
    gPartyExperience= 1000;
    gPartyGold= 1000;
    */
}

void debugEncounter(void) {
    gCurrentGameMode= gm_init;
    addNewMonster(1, 1, 3, 4, 0);
    addNewMonster(2, 1, 2, 5, 1);
    addNewMonster(0xa0, 1, 1, 1, 2);
    prepareForGameMode(gm_encounter);
    mainDispatchLoop();
}

void debugDungeon(void) {
    gCurrentDungeonIndex= 0;
    gStartXPos= 15;
    gStartYPos= 1;
    // gCurrentDungeonIndex= 1;
    // gStartXPos= 10;
    // gStartYPos= 18;
    gCurrentGameMode= gm_init;
    prepareForGameMode(gm_dungeon);
    mainDispatchLoop();
}

int main() {
    static char choice;
    char *test;
    char *mainMenu[]= {"Load saved game", NULL, NULL};

    initEngine();
    cg_clrscr();
    cg_borders(false);

    sprintf(drbuf, "Start in %s", gCities[3]);
    mainMenu[1]= drbuf;
    choice= runMenu(mainMenu, 4, 11, true, true);

    gCurrentCityIndex= 3;
    prepareForGameMode(gm_city);

    if (choice == 100) {
        debugDungeon();
    }

    if (choice == 101) {
        debugEncounter();
    }

    if (choice == 0 && hasLoadedGame) {
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
