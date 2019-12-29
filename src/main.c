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

#include <cbm.h>
#include <device.h>
#include <time.h>

#include "charset.h"
#include "irq.h"

#include "config.h"
#include "congui.h"
#include "debug.h"
#include "dungeon.h"
#include "guild.h"
#include "types.h"

#include "encounter.h"
#include "monster.h"
#include "spell.h"

#include "dispatcher.h"

char *drbuf;

byte hasLoadedGame;

void initEngine(void);
void runCityMenu(void);
void doGuild(void);
void loadSaved(void);
void installCharset(void);

void initEngine(void) {
    unsigned int rseed;

    drbuf= (char *)0xff40; // use ram at top of i/o for buffer

    cg_init();
    rseed= *(unsigned int *)0xff02; // ted free running timer for random seed
    srand(rseed);
    if (cbm_load("charset", getcurrentdevice(), (void *)0xf800) == 0) {
        puts("Failed loading charset.");
        exit(0);
    }
    installIRQ();
    hasLoadedGame= loadParty();
    enableCustomCharset();
    gLoadedDungeonIndex= 255;
}

void debugEncounter(void) {
    clearMonsters();
    // gCurrentGameMode= gm_dungeon;
    gCurrentDungeonIndex= 0;
    addNewMonster(0, 1, 6, 0);
    addNewMonster(1, 1, 3, 1);
    addNewMonster(2, 1, 1, 2);
    prepareForGameMode(gm_dungeon);
    mainDispatchLoop();
}

int main() {
    static char choice;
    byte i;

    initEngine();
    clrscr();
    gotoxy(0,0);
    puts("Dragon Rock I");
    gotoxy(28,0);
    puts("Version 0.1a");
    gotoxy(30,1);
    puts("12/29/2019");

    // TESTING
    for (i= 0; i < 5; i++) {
        setHasSpell(party[0], i);
        setHasSpell(party[1], i + 5);
        party[i]->gold= 100;
    }

    cputsxy(2, 12, "1 - load saved game");
    cputsxy(2, 14, "2 - start in ");
    cputs(gCities[0]);

    gCurrentCityIndex= 0;
    prepareForGameMode(gm_city);

    do {
        choice= cgetc();
    } while (strchr("12d", choice) == NULL);

    if (choice == 'd') {
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
