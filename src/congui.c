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

char *waitChars= "/-\\|";
byte waitIdx;

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
    byte i;
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
    unsigned int bas;
    bas = y*40;
    lfill ((long)SCREEN+bas+x0,character,x1-x0+1);
    lfill ((long)COLOR_RAM+bas+x0,col,x1-x0+1);
}

void cg_block(byte x0, byte y0, byte x1, byte y1, byte character, byte col) {
    byte i,num;
    for (i= y0; i <= y1; ++i) {
        cg_line(i,x0,x1,character,col);
    }
}

void cg_verticalMenu(byte x0, byte y0, byte col, char *items[]) {
    byte x,y;
    byte currentNum;
    char *currentItem;
    currentNum=0;
    currentItem = items[0];
    revers(1);
    textcolor(col);
    while (currentItem[0]!='#') {
        gotoxy(x0,y0+currentNum);
        cputs(currentItem);
        currentItem = items[++currentNum];
    }
}

void cg_init() {
    bgcolor(COLOR_BLACK);
    bordercolor(COLOR_BLACK);
    textcolor(COLOR_GREEN);
    cbm_k_bsout(11); // disable shift+cmd on c128 & 364
    cbm_k_bsout(14); // lowercase charset
    cg_clear();
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
