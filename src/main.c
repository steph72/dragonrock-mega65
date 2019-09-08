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
#include "guildLoader.h"
#include "types.h"

#include "encounter.h"
#include "monster.h"

extern void _OVERLAY1_LOAD__[], _OVERLAY1_SIZE__[];
extern void _OVERLAY2_LOAD__[], _OVERLAY2_SIZE__[];

byte currentCity;
byte hasLoadedGame;
char outbuf[80];

void initEngine(void);
void runCityMenu(void);
void runGuildMenu(void);
void loadSaved(void);
void installCharset(void);

unsigned char loadfile(char *name, void *addr, void *size);

void initEngine(void) {
    unsigned int rseed;
    const char prompt[]= "ARCHAIC(tm) engine for TED/64k\n"
                         "Version 0.1 alpha\n\n"
                         "Written by Stephan Kleinert\n"
                         "at K-Burg, Bad Honnef, and\n"
                         "at Hundehaus im Reinhardswald\n\n"
                         "Copyright (c) 2019 7Turtles Software\n";
    cg_init();
    puts(prompt);
    rseed= *(unsigned int *)0xff02; // ted free running timer for random seed
    srand(rseed);
    if (cbm_load("charset", getcurrentdevice(), (void *)0xf400) == 0) {
        puts("Failed loading charset.");
        exit(0);
    }
    copychars();
    installIRQ();
    hasLoadedGame= initGuild();
    enableCustomCharset();
}

int main() {
    static char choice;

    initEngine();
    clrscr();
    cg_borders();
    cputsxy(2, 4, "Welcome to");
    cputsxy(5, 6, "Dragon Rock 1 - The Escape");
    cputsxy(2, 12, "1 - load saved game");
    cputsxy(2, 14, "2 - start in ");
    cputs(gCities[0]);

    currentCity= 0;

    do {
        choice= cgetc();
    } while (strchr("12d", choice) == NULL);

    if (choice == '1' && hasLoadedGame) {
        // determine last city from saved party
        for (choice= 0; choice < PARTYSIZE; choice++) {
            if (party[choice]) {
                currentCity= party[choice]->city;
                break;
            }
        }
    } else if (choice == 'd') {
        cputs("\r\n--DEBUG--");
        clearMonsters();
        addNewMonster(0, 1, 3, 0);
        addNewMonster(1, 1, 4, 1);
        addNewMonster(2, 1, 5, 2);
        doEncounter();
        gotoxy(0, 0);
        loadfile("dungeon", _OVERLAY1_LOAD__, _OVERLAY1_SIZE__);
        testMap();
    } else {
        // remove saved party if not loading saved game
        for (choice= 0; choice < PARTYSIZE; party[choice++]= 0)
            ;
    }

    loadfile("city", _OVERLAY2_LOAD__, _OVERLAY2_SIZE__);
    runCityMenu();

    return 0;
}

void installCharset() {
    byte *tedCharSrc= (byte *)0xff12;
    byte *tedCharAdr= (byte *)0xff13;
    *tedCharSrc= (*tedCharSrc) & 251;
    *tedCharAdr= 0xf0;
}

unsigned char loadfile(char *name, void *addr, void *size) {
    /* Avoid compiler warnings about unused parameters. */
    (void)addr;
    (void)size;
#ifdef DEBUG
    cprintf("\r\nov %s $%x @ $%x ", name, size, addr);
    cprintf("$%x rem", 0x2000 - (int)size);
#endif
    disableCustomCharset(); // see explanation in guildLoader why we seem to need this...
    if (cbm_load(name, getcurrentdevice(), NULL) == 0) {
        cputs("Loading overlay file failed");
        exit(0);
    }
    enableCustomCharset();
    return 1;
}
