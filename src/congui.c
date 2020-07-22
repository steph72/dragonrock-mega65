/*
 * congui.c
 *
 * Copyright (C) 2019 - Stephan Kleinert
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "globals.h"
#include "memory.h"
#include <c64.h>
#include <cbm.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static byte gPal;
static signed char gPalDir;

static clock_t lastPaletteTick;

void cg_emptyBuffer(void) {
    while (kbhit()) {
        cgetc();
    }
}

char cg_getkey(void) {
    cg_emptyBuffer();
    return cgetc();
}

void cg_clearFromTo(byte start, byte end) {
    static byte i;
    for (i= start; i < end; ++i) {
        cclearxy(0, i, 40);
    }
}

void cg_clearLower(byte num) { cg_clearFromTo(24 - num, 24); }

void cg_clear(void) {
    lfill((long)SCREEN, 32, 1000);
    lfill((long)COLOR_RAM, 0, 1000);
    gotoxy(0, 0);
}

void cg_line(byte y, byte x0, byte x1, byte character, byte col) {
    static unsigned int bas;
    bas= y * 40;
    lfill((long)SCREEN + bas + x0, character, x1 - x0 + 1);
    lfill((long)COLOR_RAM + bas + x0, col, x1 - x0 + 1);
}

void cg_block(byte x0, byte y0, byte x1, byte y1, byte character, byte col) {
    byte i;
    for (i= y0; i <= y1; ++i) {
        cg_line(i, x0, x1, character, col);
    }
}

void cg_center(byte x, byte y, byte width, char *text) {
    byte mid;
    mid= width / 2;
    gotoxy(x + (width / 2) - (strlen(text) / 2), y);
    cputs(text);
}

byte cg_verticalList(byte x0, byte y0, byte lineSpacing, byte width, byte col,
                     char *items[]) {
    byte currentNum;
    char *currentItem;

    currentNum= 0;
    currentItem= items[0];
    textcolor(col);
    revers(1);
    while (currentItem[0] != '\0') {
        if (width != 0) {
            cg_center(x0, y0 + (currentNum * lineSpacing), width, currentItem);
        } else {
            gotoxy(x0, y0 + (currentNum * lineSpacing));
            cputs(currentItem);
        }
        currentItem= items[++currentNum];
    }
    return currentNum;
}

void cg_stepColor(void) {
    if ((clock() - lastPaletteTick) < 4) {
        return;
    }
    lastPaletteTick= clock();
    gPal+= gPalDir;
    POKE(0xd100U + 1, gPal/2);
    POKE(0xd200U + 1, gPal);
    POKE(0xd300U + 1, 15);
    if (gPal == 15) {
        gPalDir= -1;
    } else if (gPal == 0) {
        gPalDir= 1;
    }
}

void cg_stopColor(void) {
    POKE(0xd100U + 1, 15);
    POKE(0xd200U + 1, 15);
    POKE(0xd300U + 1, 15);
}

byte cg_verticalMenu(byte x0, byte y0, byte lineSpacing, byte width,
                     byte menuItemCount) {
    static byte originalColor;
    static byte currentRow;
    static byte *currentBase;
    static byte lineInc;
    static char input;

    currentRow= 0;

    do {
        currentBase= COLOR_RAM + ((y0 + (lineSpacing * currentRow)) * 40) + x0;
        originalColor= *currentBase;
        lfill((long)currentBase, 1, width);
        while (!kbhit()) {
            cg_stepColor();
        }
        input= cgetc();
        lfill((long)currentBase, originalColor, width);
        switch (input) {
        case 17: // down
            if (currentRow < menuItemCount - 1)
                currentRow++;
            break;
        case 145: // up
            if (currentRow > 0)
                currentRow--;
        default:
            break;
        }
    } while (input != 13);
    cg_stopColor();
    return currentRow;
}

byte cg_menu(byte width, char *items[] ) {
    byte x,y;
    byte numItems;
    x = wherex();
    y = wherey();
    numItems = cg_verticalList(x,y,1,width,COLOR_GRAY2,items);
    return cg_verticalMenu(x,y,1,width,numItems);
}

void cg_init() {
    bgcolor(COLOR_BLACK);
    bordercolor(COLOR_BLACK);
    textcolor(COLOR_GREEN);
    gPal= 0;
    gPalDir= 1;
    cbm_k_bsout(11); // disable shift+cmd on c128 & 364
    cbm_k_bsout(14); // lowercase charset
    cg_clear();
    mega65_io_enable();
    POKE(0xd030U, PEEK(0xd030U) | 4); // enable palette
}

void cg_borders(void) {
    chlinexy(0, 0, 40);
    chlinexy(0, 24, 40);
    gotoxy(0, 1);
}

void cg_titlec(byte lcol, byte tcol, byte splitScreen, char *t) {
    byte xpos;
    byte splitPos= 12;

    xpos= 20 - (strlen(t) / 2);
    cg_clear();
    textcolor(lcol);
    cg_borders();
    if (splitScreen) {
        if (splitScreen != true) {
            splitPos= splitScreen;
        }
        chlinexy(0, splitPos, 40);
    }
    cputsxy(xpos, 0, t);
    cputcxy(xpos - 1, 0, 32);
    cputcxy(xpos + strlen(t), 0, 32);

    textcolor(tcol);
    gotoxy(0, 3);
}
