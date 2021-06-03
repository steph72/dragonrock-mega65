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

#include "globals.h"

#include "congui.h"
#include "debug.h"
#include "dungeon.h"
#include "guild.h"

#include "encounter.h"
#include "monster.h"
#include "spell.h"

#include "c65.h"
#include "dispatcher.h"
#include "memory.h"
#include "sprites.h"

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

char drbuf[BUFSIZE];
char textbuf[TEXTBUF_SIZE];

byte hasLoadedGame;

void initEngine(void);
void runCityMenu(void);
void doGuild(void);
void loadSaved(void);

const char *prompt=
    "drenigne/m65 v" DRE_VERSION " build " DRE_BUILDNUM "\n" DRE_DATE "\n\n";

void loadCharset(void) {
    byte *charTemp;
    charTemp= (byte *)malloc(4096);
    if (cbm_load("charset", getcurrentdevice(), (void *)charTemp) == 0) {
        puts("Failed loading charset.");
        exit(0);
    }
    lcopy((long)charTemp, 0xff7e800U, 4096); // directly overwrite charset
    free(charTemp);
}

void initVIC() {
    byte *vic3_control= (byte *)0xd031;
    byte *hotreg= (byte *)0xd05d;

    POKE(0xd02fL, 0x47);
    POKE(0xd02fL, 0x53);
    *vic3_control&= (255 ^ 128); // disable 80chars
    *vic3_control&= (255 ^ 8);   // disable interlace
    *hotreg&= 127;               // disable hotreg
    POKE(53297L, 96);
}

void initEngine(void) {

    unsigned char i;
    srand(42);
    puts(prompt);
    puts("initializing engine.\n");
    loadModules();
    puts("init monster rows\n");
    initMonsterRows();
    puts("load charset\n");
    loadCharset();
    puts("init sprites\n");
    initSprites();
    puts("load items\n");
    loadExt("items", ITEM_BASE);
    puts("init party\n");
    hasLoadedGame= loadParty();
    gLoadedDungeonIndex= 255;
    gCurrentGameMode= gm_init;
    cg_getkey();
    initVIC();
    cg_init();

    /*
    gPartyExperience= 1000;
    gPartyGold= 1000;
    */
}

void debugEncounter(void) {
    gCurrentGameMode= gm_init;
    addNewMonster(1, 1, 3, 0);
    addNewMonster(1, 1, 4, 1);
    addNewMonster(1, 1, 5, 2);
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
    initEngine();
    cg_clear();
    gotoxy(0, 2);
    cputsxy(2, 11, "1 - load saved game");
    cputsxy(2, 13, "2 - start in ");
    cputs(gCities[0]);

    gCurrentCityIndex= 0;
    prepareForGameMode(gm_city);

    do {
        choice= cgetc();
    } while (strchr("12de", choice) == NULL);

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
