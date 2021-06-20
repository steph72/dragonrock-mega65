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
byte devmode;

void initEngine(void);
void runCityMenu(void);
void doGuild(void);
void loadSaved(void);

const char *prompt=
    "DREngine/m65 v" DRE_VERSION " build " DRE_BUILDNUM "\n" DRE_DATE "\n\n";

void initVIC() {
    byte *vic3_control= (byte *)0xd031;
    byte *hotreg= (byte *)0xd05d;

    POKE(0xd02fL, 0x47);
    POKE(0xd02fL, 0x53);
    //*vic3_control&= (255 ^ 128); // disable 80chars
    //*vic3_control&= (255 ^ 8);   // disable interlace
    //*hotreg&= 127;               // disable hotreg
    POKE(53297L, 96); // quit bitplane mode
}

void loadResources(void) {
    unsigned int readBytes;
    printf("load items; ");
    readBytes= loadExt("items", ITEM_BASE);
    printf("$%x bytes read\n", readBytes);
    printf("load monsters; ");
    readBytes= loadExt("monsters", MONSTERS_BASE);
    printf("$%x bytes read\n", readBytes);
}

void initEngine(void) {

    mega65_io_enable();
    srand(DRE_BUILDNUM);
    puts("\n");      // cancel leftover quote mode from wrapper or whatever
    cbm_k_bsout(14); // lowercase
    clrscr();
    puts(prompt);

    lcopy(0x5f000, (long)drbuf, 4);
    if (drbuf[0] == 0x53 && drbuf[1] == 0x4b) {
        devmode= true;
    } else if (drbuf[0] != 0x23 || drbuf[1] != 0x45) {
        initVIC();
        puts("Please BOOT the dragon rock disc to");
        puts("correctly initialize the game.");
        while (1)
            ;
    }

    drbuf[0]= 0;
    drbuf[1]= 0;
    lcopy((long)drbuf, 0x5f000, 4);

    loadModules();
    puts("init monster rows");
    initMonsterRows();
    puts("init sprites");
    initSprites();
    loadResources();
    puts("init party");
    hasLoadedGame= loadParty();
    gLoadedDungeonIndex= 255;
    gCurrentGameMode= gm_init;
    if (devmode) {
        puts("\ninitialization complete");
        // puts("** development mode. press any key **");
        // cg_getkey();
    }

    initVIC();
    cg_init();
    cg_go16bit();
    cg_clrscr();
    cg_puts("Outside!");
    cg_gotoxy(13,13);
    cg_puts("Outside2!");
    cg_setwin(3,3,10,10);
    cg_gotoxy(0,0);
    cg_puts("!1234567892\n3\n4\n");
    cg_printf("Hello world %x %d\nThe quick brown Candor jumps\nover the lazy Buba!\n", 1234, 5678);
    cg_textcolor(COLOR_RED);
    cg_printf("This should be red!");
    cg_getkey();
    cg_clrscr();
    cg_getkey();
    cg_go8bit();

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
    initEngine();
    cg_clrscr();
    gotoxy(0, 2);
    cputsxy(2, 11, "1 - load saved game");
    cputsxy(2, 13, "2 - start in ");
    cputs(gCities[0]);

    gCurrentCityIndex= 0;
    prepareForGameMode(gm_city);

    choice= cgetc();
    if (strchr("12de", choice) == NULL) {
        choice= '1';
    }

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
