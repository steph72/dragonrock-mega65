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

#include "congui.h"
#include "globals.h"
#include "memory.h"
#include "utils.h"
#include <c64.h>
#include <cbm.h>
#include <conio.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static byte gPal;
static signed char gPalDir;
static clock_t lastPaletteTick;


textwin currentWin;

unsigned char drColours[16][3]= {
    {0x00, 0x00, 0x00}, // black
    {0xff, 0xff, 0xff}, // white
    {0x68, 0x37, 0x2b}, // red
    {0x70, 0xa4, 0xb2}, // cyan
    {0x6f, 0x3d, 0x86}, // purple
    {0x68, 0x9d, 0x43}, // green
    {0x35, 0x28, 0x79}, // 06
    {0xb8, 0xc7, 0x6f}, // 07
    {0xf0, 0xa0, 0x00}, // 08
    {0x43, 0x39, 0x00}, // 09
    {0xfa, 0x27, 0x39}, // 0a
    {0x44, 0x44, 0x44}, // 11
    {0x6c, 0x6c, 0x6c}, // 12
    {0x9a, 0xd2, 0x84}, // 13
    {0x6c, 0x5e, 0xb5}, // 14
    {0x95, 0x95, 0x95}  // 15
};

#define SCREENBASE 0x12000l
#define COLBASE 0xff80000l
#define SHAPEBASE 0x50000l

#define SHAPETBLSIZE 16

#define VIC4CTRL (*(unsigned char *)(0xd054))
#define VIC3CTRL (*(unsigned char *)(0xd031))
#define LINESTEP_LO (*(unsigned char *)(0xd058))
#define LINESTEP_HI (*(unsigned char *)(0xd059))
#define CHRCOUNT (*(unsigned char *)(0xd05e))
#define HOTREG (*(unsigned char *)(0xd05d))

#define SCNPTR_0 (*(unsigned char *)(0xd060))
#define SCNPTR_1 (*(unsigned char *)(0xd061))
#define SCNPTR_2 (*(unsigned char *)(0xd062))
#define SCNPTR_3 (*(unsigned char *)(0xd063))

byte is16BitModeEnabled;
byte xc16, yc16;
byte textcolor16;
unsigned long gScreenSize;
byte gScreenColumns;
byte gScreenRows;

himemPtr shapeTbl[SHAPETBLSIZE];
himemPtr nextFreeShapeMem;

void scrollUp();

void cg_clearShapes() {
    byte i;
    for (i= 0; i < SHAPETBLSIZE; ++i) {
        shapeTbl[i]= 0;
    }
    nextFreeShapeMem= SHAPEBASE;
}

void cg_test() {
    word foo;
    cg_go16bit(0, 0);
    cg_clrscr();
    cg_displayDBMFile("foo.dbm",0,0);
    // foo= cg_loadDBM("foo.dbm", 0, 0, 0x50000);
    cg_getkey();
    cg_displayDBMFile("foo2.dbm",1,1);
    // foo= cg_loadDBM("foo2.dbm", 1, 1, 0x40000);
    cg_gotoxy(1, 1);
    cg_printf("This is a test. nextFree = %x", foo);
    cg_getkey();
    cg_gotoxy(13, 13);
    cg_puts("Outside!");
    cg_setwin(20, 2, 15, 15);
    cg_gotoxy(0, 0);
    cg_puts("!1234567892\n3\n4\n");
    cg_printf("Hello world %x %d\nThe quick brown Candor jumps\nover the lazy "
              "Buba!\n",
              1234, 5678);
    cg_textcolor(COLOR_RED);
    cg_printf("This should be red!");
    for (foo= 0; foo < 5; ++foo) {
        cg_getkey();
        scrollUp();
    }
    cg_setwin(0, 0, 40, 25);
    for (foo= 0; foo < 5; ++foo) {
        cg_getkey();
        scrollUp();
    }
    cg_go8bit();
}

char asciiToPetscii(byte c) {
    if (c >= 65 && c <= 95) {
        return c - 64;
    }
    if (c >= 193) {
        return c - 128;
    }
    return c;
}

void cg_go16bit(byte h640, byte v400) {
    mega65_io_enable();
    VIC4CTRL|= 0x04; // enable full colour for characters with high byte set
    VIC4CTRL|= 0x01; // enable 16 bit characters

    if (h640) {
        VIC3CTRL|= 0x80; // enable H640
        gScreenSize= 2000;
        gScreenColumns= 80;
    } else {
        VIC3CTRL&= 0x7f; // disable H640
        gScreenSize= 1000;
        gScreenColumns= 40;
    }

    if (v400) {
        VIC3CTRL|= 0x08;
        gScreenRows= 50;
        gScreenSize*= 2;
    } else {
        gScreenRows= 25;
        VIC3CTRL&= 0xf7;
    }

    HOTREG&= 127; // disable hotreg
    CHRCOUNT= gScreenColumns;
    LINESTEP_LO= gScreenColumns * 2;
    LINESTEP_HI= 0;

    SCNPTR_0= SCREENBASE & 0xff; // screen to 0x10000
    SCNPTR_1= (SCREENBASE >> 8) & 0xff;
    SCNPTR_2= (SCREENBASE >> 16) & 0xff;
    SCNPTR_3&= 0xF0 | ((SCREENBASE) << 24 & 0xff);
    lfill(SCREENBASE, 0, gScreenSize * 2);
    lfill(COLBASE, 0, gScreenSize * 2);
    xc16= 0;
    yc16= 0;
    textcolor16= 5;
    is16BitModeEnabled= true;
    currentWin.x0= 0;
    currentWin.x1= gScreenColumns - 1;
    currentWin.y0= 0;
    currentWin.y1= gScreenRows - 1;
    currentWin.width= gScreenColumns;
    currentWin.height= gScreenRows;
}

void cg_go8bit() {
    mega65_io_enable();
    SCNPTR_0= 0x00; // screen back to 0x800
    SCNPTR_1= 0x08;
    SCNPTR_2= 0x00;
    SCNPTR_3&= 0xF0;
    VIC4CTRL&= 0xFA; // clear fchi and 16bit chars
    CHRCOUNT= 40;
    LINESTEP_LO= 40;
    LINESTEP_HI= 0;
    HOTREG|= 0x80;   // enable hotreg
    VIC3CTRL&= 0x7f; // disable H640
    VIC3CTRL&= 0xf7; // disable V400
    cbm_k_bsout(14); // lowercase charset
    is16BitModeEnabled= false;
}

void cg_addGraphicsRect(byte x0, byte y0, byte width, byte height,
                        himemPtr bitmapData) {
    byte x, y;
    long adr;
    word currentCharIdx;

    currentCharIdx= bitmapData / 64;

    for (y= y0; y < y0 + height; ++y) {
        for (x= x0; x < x0 + width; ++x) {
            adr= SCREENBASE + (x * 2) + (y * gScreenColumns * 2);
            lpoke(adr, currentCharIdx % 256);
            lpoke(adr + 1, currentCharIdx / 256);
            currentCharIdx++;
        }
    }
}

void cg_loadDBM(char *filename, himemPtr adr, dbmInfo *info) {

    FILE *dbmfile;
    byte i;
    byte r, g, b;
    byte numColumns, numRows, numColours;
    byte dbmOptions;
    byte *palette;
    word palsize;
    word imgsize;
    word colAdr;
    word bytesRead;
    dbmInfo retInfo;

    byte reservedSysPalette;

    dbmfile= fopen(filename, "rb");
    if (!dbmfile) {
        puts("can't load ");
        puts(filename);
        while (1)
            ;
    }
    fread(drbuf, 1, 12, dbmfile);
    if (0 != memcmp(drbuf, "dbmp", 4)) {
        puts("not a dbm file");
        while (1)
            ;
    }
    numRows= drbuf[5];
    numColumns= drbuf[6];
    dbmOptions= drbuf[7];
    numColours= drbuf[8];
    reservedSysPalette= dbmOptions & 2;

    if (0 != memcmp(&drbuf[9], "pal", 3)) {
        puts("missing pal entry");
        while (1)
            ;
    }

    palsize= numColours * 3;
    palette= (byte *)malloc(palsize);
    fread(palette, 3, numColours, dbmfile);

    mega65_io_enable();
    for (i= 0; i < numColours; ++i) {
        if (reservedSysPalette && i < 15) {
            continue;
        }
        colAdr= i * 3;
        r= palette[colAdr];
        g= palette[colAdr + 1];
        b= palette[colAdr + 2];
        POKE(0xd100u + i, r);
        POKE(0xd200u + i, g);
        POKE(0xd300u + i, b);
    }
    free(palette);
    imgsize= numColumns * numRows * 64;

    bytesRead= readExt(dbmfile, adr);
    fclose(dbmfile);
    mega65_io_enable();

    if (info != NULL) {
        info->columns= numColumns;
        info->rows= numRows;
        info->size= bytesRead;
        info->baseAdr= adr;
    }
}

void cg_displayDBMInfo(dbmInfo *info, byte x0, byte y0) {
    mega65_io_enable();
    cg_addGraphicsRect(x0,y0,info->columns,info->rows,info->baseAdr);
}

void cg_displayDBMFile(char *filename, byte x0, byte y0) {
    dbmInfo info;
    // TODO: manage addresses
    cg_loadDBM(filename, 0x40000, &info);
    cg_displayDBMInfo(&info, x0, y0);
}

void scrollUp() {
    byte y;
    long bas0, bas1;
    for (y= currentWin.y0; y < currentWin.y1; y++) {
        bas0= SCREENBASE + (currentWin.x0 * 2 + (y * gScreenColumns * 2));
        bas1= SCREENBASE + (currentWin.x0 * 2 + ((y + 1) * gScreenColumns * 2));
        lcopy(bas1, bas0, currentWin.width * 2);
        bas0= COLBASE + (currentWin.x0 * 2 + (y * gScreenColumns * 2));
        bas1= COLBASE + (currentWin.x0 * 2 + ((y + 1) * gScreenColumns * 2));
        lcopy(bas1, bas0, currentWin.width * 2);
    }
    cg_block_raw(currentWin.x0, currentWin.y1, currentWin.x1, currentWin.y1, 32,
                 textcolor16);
}

void cr() {
    xc16= currentWin.x0;
    yc16++;
    if (yc16 > currentWin.y1) {
        yc16= currentWin.y1;
        scrollUp();
    }
}

void outc16(char c) {
    char out;
    word adrOffset;
    if (c == '\n') {
        cr();
        return;
    }
    out= asciiToPetscii(c);
    adrOffset= (xc16 * 2) + (yc16 * 2 * gScreenColumns);
    lpoke(SCREENBASE + adrOffset, out);
    lpoke(SCREENBASE + adrOffset + 1, 0);
    lpoke(COLBASE + adrOffset + 1, textcolor16);
    lpoke(COLBASE + adrOffset, 0);
    xc16++;
    if (xc16 > currentWin.x1) {
        yc16++;
        xc16= currentWin.x0;
        if (yc16 > currentWin.y1) {
            yc16= currentWin.y1;
            scrollUp();
        }
    }
}

void cg_puts(char *s) {
    char *current= s;
    while (*current) {
        outc16(*current++);
    }
}

void box16(byte x0, byte y0, byte x1, byte y1, byte b, byte c) {
    int x, y;
    word adrOffset;
    for (x= x0; x <= x1; ++x) {
        for (y= y0; y <= y1; ++y) {
            adrOffset= x * 2 + (y * 2 * gScreenColumns);
            lpoke(SCREENBASE + adrOffset, b);
            lpoke(SCREENBASE + adrOffset + 1, 0);
            lpoke(COLBASE + adrOffset + 1, c);
        }
    }
}

void cg_textcolor(byte c) { textcolor16= c; }

void cg_gotoxy(byte x, byte y) {
    if (is16BitModeEnabled) {
        xc16= currentWin.x0 + x;
        yc16= currentWin.y0 + y;
        return;
    }
    gotoxy(x, y);
}

int cg_printf(const char *format, ...) {
    char buf[160];
    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    cg_puts(buf);
}

void cg_clrscr() {
    if (is16BitModeEnabled) {
        cg_block_raw(currentWin.x0, currentWin.y0, currentWin.x1, currentWin.y1,
                     32, 5);
        return;
    }
    lfill((long)SCREEN, 32, 1000);
    lfill((long)COLOR_RAM, 0, 1000);
    cg_gotoxy(0, 0);
}

void cg_setwin(byte x0, byte y0, byte width, byte height) {
    currentWin.x0= x0;
    currentWin.y0= y0;
    currentWin.x1= x0 + width - 1;
    currentWin.y1= y0 + height - 1;
    currentWin.width= width;
    currentWin.height= height;
    cg_gotoxy(0, 0);
}

void cg_init() {
    byte i;
    mega65_io_enable();
    POKE(53297U, 96);                 // quit bitplane mode
    POKE(0xd030U, PEEK(0xd030U) | 4); // enable palette
    for (i= 0; i < 15; ++i) {
        cg_setPalette(i, drColours[i][0], drColours[i][1], drColours[i][2]);
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

void cg_line(byte y, byte x0, byte x1, byte character, byte col) {
    static word bas;
    static word bas2;
    word i;
    if (is16BitModeEnabled) {
        bas= x0 * 2 + (y * gScreenColumns * 2);
        bas2= x1 * 2 + (y * gScreenColumns * 2);
        for (i= bas; i <= bas2; i+= 2) {
            lpoke(SCREENBASE + i, character);
            lpoke(SCREENBASE + i + 1, 0);
            lpoke(COLBASE + i, 0);
            lpoke(COLBASE + i + 1, col);
        }
        return;
    }
    bas= y * 40;
    if (character) {
        lfill((long)SCREEN + bas + x0, character, x1 - x0 + 1);
    }
    lfill((long)COLOR_RAM + bas + x0, col, x1 - x0 + 1);
}

void cg_block_raw(byte x0, byte y0, byte x1, byte y1, byte character,
                  byte col) {
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
    cg_clrscr();
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
