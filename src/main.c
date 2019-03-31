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

#include <conio.h>
#include <em.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cbm.h>
#include <device.h>

#include "config.h"
#include "congui.h"
#include "dungeon.h"
#include "guild.h"
#include "types.h"

#define debug 1

extern void _OVERLAYADDR__[], _OVERLAYSIZE__[];

byte currentCity;
byte hasLoadedGame;
char outbuf[80];

struct {
    char *name;
    void *addr;
    unsigned size;
} overlay[]= {{"city", _OVERLAYADDR__, (unsigned)_OVERLAYSIZE__},
              {"dungeon", _OVERLAYADDR__, (unsigned)_OVERLAYSIZE__},
              {"outdoor", _OVERLAYADDR__, (unsigned)_OVERLAYSIZE__}};

void initEngine(void);
void runCityMenu(void);
void runGuildMenu(void);
void loadSaved(void);

unsigned char loademdriver(void);
unsigned char loadoverlay(unsigned char num);

void initEngine(void) {
    const char prompt[]= "ARCHAIC(tm) engine for the C128\n"
                         "Version 0.1 alpha\n"
                         "Written by Stephan Kleinert\n"
                         "Copyright (c) 2019 7Turtles Software\n";
    cg_init();
    puts(prompt);

    cputs("loading em driver... ");
    if (loademdriver()) {
        cputs("ok\r\n");
    } else {
        cputs("error!\r\n");
        exit(0);
    }

    testMap();
    exit(0);

    cputs("loading city... ");
    if (loadoverlay(1)) {
        cputs("ok\r\n");
    } else {
        cputs("error!\r\n");
        exit(0);
    }

    cputs("loading guild... ");
    hasLoadedGame= initGuild(); // need to load guild here
    if (!hasLoadedGame) {       // since initGuild() is in city overlay
        cputs("not found\r\n");
    } else {
        cputs("ok\r\n");
    }
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
    } while (strchr("12", choice) == NULL);

    if (choice == '1' && hasLoadedGame) {
        // determine last city from saved party
        for (choice= 0; choice < PARTYSIZE; choice++) {
            if (party[choice]) {
                currentCity= party[choice]->city;
                break;
            }
        }
    } else {
        // remove saved party if not loading saved game
        for (choice= 0; choice < PARTYSIZE; party[choice++]= 0)
            ;
    }

    runCityMenu();

    return 0;
}

unsigned char loademdriver(void) {
    if (em_load_driver("em") == EM_ERR_OK) {
        return 1;
    }
    return 0;
}

unsigned char loadoverlay(unsigned char num) {
    if (cbm_load(overlay[num - 1].name, getcurrentdevice(), NULL) == 0) {
        cputs("Loading overlay file failed");
        return 0;
    }
    return 1;
}