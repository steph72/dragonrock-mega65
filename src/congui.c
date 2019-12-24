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

#include "types.h"
#include <cbm.h>
#include <conio.h>
#include <plus4.h>
#include <stdio.h>
#include <string.h>

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
        cclearxy(0,i,40);
    }
}

void cg_clearLower(byte num) {
    cg_clearFromTo(24-num,24);
}

void cg_clearGraphics(void) {
    char *hiadr= (char *)0xc00; // graphics area
    memset(hiadr, 0xff, 680);   // clear 17 lines with 0xff
}

void cg_clearText(void) {
    char *loadr= (char *)0xea8; // text area
    memset(loadr, 0x20, 320);   // clear 8 lines with 0x20
}

void cg_clear(void) {
    cg_clearGraphics();
    cg_clearText();
}

void cg_init() {
    bgcolor(COLOR_BLACK);
    bordercolor(COLOR_BLACK);
    textcolor(BCOLOR_LEMON + CATTR_LUMA5);
    cbm_k_bsout(11);          // disable shift+cmd on c128 & 364
    cbm_k_bsout(14);          // lowercase charset
    bzero((byte *)0xf7f8, 8); // create empty space for graphics screen
    clrscr();
}

void cg_borders(void) {
    chlinexy(0, 0, 40);
    chlinexy(0, 24, 40);
    gotoxy(0, 1);
}

void cg_titlec(byte lcol, byte tcol, byte splitScreen, char *t) {
    clrscr();
    textcolor(lcol);
    cg_borders();
    if (splitScreen)
        chlinexy(0, 12, 40);
    textcolor(tcol);
    cputsxy(20 - (strlen(t) / 2), 1, t);
    gotoxy(0, 3);
}
