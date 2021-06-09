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

unsigned char drColours[16][3]= {
    {0x00, 0x00, 0x00}, // black
    {0xff, 0xff, 0xff}, // white
    {0x68, 0x37, 0x2b}, // red
    {0x70, 0xa4, 0xb2}, // cyan
    {0x6f, 0x3d, 0x86}, // purple
    {0x68, 0x9d, 0x43}, // green
    {0x35, 0x28, 0x79}, // 06
    {0xb8, 0xc7, 0x6f}, // 07
    {0xb0, 0x60, 0x15}, // 08
    {0x43, 0x39, 0x00}, // 09
    {0xfa, 0x27, 0x39}, // 0a
    {0x44, 0x44, 0x44}, // 11
    {0x6c, 0x6c, 0x6c}, // 12
    {0x9a, 0xd2, 0x84}, // 13
    {0x6c, 0x5e, 0xb5}, // 14
    {0x95, 0x95, 0x95}  // 15
};

char cg_getkey(void);
void cg_setPalette(byte num, byte red, byte green, byte blue);

void cg_init() {
    byte i;
    mega65_io_enable();
    POKE(0xd030U, PEEK(0xd030U) | 4); // enable palette
    for (i=0;i<15;++i) {
        cg_setPalette(i,drColours[i][0],drColours[i][1],drColours[i][2]);
    }
    bgcolor(COLOR_BLACK);
    bordercolor(COLOR_BLACK);
    textcolor(COLOR_GREEN);
    cbm_k_bsout(13);
    clrscr();
    gPal= 0;
    gPalDir= 1;
    cbm_k_bsout(11); // disable shift+cmd on c128 & 364
    cbm_k_bsout(14); // lowercase charset
}

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

void cg_colorLine(byte y, byte x0, byte x1, byte col) {
    static unsigned int bas;
    bas= y * 40;
    lfill((long)COLOR_RAM + bas + x0, col, x1 - x0 + 1);
}

void cg_line(byte y, byte x0, byte x1, byte character, byte col) {
    static unsigned int bas;
    bas= y * 40;
    if (character) {
        lfill((long)SCREEN + bas + x0, character, x1 - x0 + 1);
    }
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


unsigned char nyblswap(unsigned char in) // oh why?!
{
  return ((in & 0xf) << 4) + ((in & 0xf0) >> 4);
}

void cg_setPalette(byte num, byte red, byte green, byte blue) {
    POKE(0xd100U + num, nyblswap(red));
    POKE(0xd200U + num, nyblswap(green));
    POKE(0xd300U + num, nyblswap(blue));
}

void cg_stepColor(void) {
    if ((clock() - lastPaletteTick) < 3) {
        return;
    }
    lastPaletteTick= clock();
    gPal+= gPalDir;
    POKE(0xd100U + 1, gPal / 2);
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

char cg_getkeyP(byte x, byte y, const char *prompt) {
    cg_emptyBuffer();
    gotoxy(x, y);
    textcolor(COLOR_WHITE);
    cputs(prompt);
    while (!kbhit()) {
        cg_stepColor();
    }
    cg_stopColor();
    return cgetc();
}


char *cg_input(byte maxlen) {
    // TODO
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
