/*
 * congui.c
 * console, graphics and graphical user interface module for the MEGA65
 *
 * Copyright (C) 2019-21 - Stephan Kleinert
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
#include "debug.h"
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

#ifdef DEBUG
#include "debug.h"
#endif

static byte gPal;
static signed char gPalDir;
clock_t lastPaletteTick;

textwin currentWin;
textwin *saveWin= 0x0600;
byte winCount= 0;

#define MAX_DBM_BLOCKS 16
#define SHAPETBLSIZE 16

#define VIC_BASE 0xD000UL

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

#define COLOUR_RAM_OFFSET COLBASE - 0xff80000l

// special graphics characters
#define H_COLUMN_END 4
#define H_COLUMN_START 5

#define CURSOR_CHARACTER 0x5f

byte textcolor16;          // text colour
unsigned long gScreenSize; // screen size (in characters)
byte gScreenColumns;       // number of screen columns (in characters)
byte gScreenRows;          // number of screen rows (in characters)
himemPtr
    nextFreeGraphMem; // location of next free graphics block in banks 4 & 5
himemPtr nextFreePalMem;
byte infoBlockCount; // number of info blocks

byte cgi;     // universal loop var
byte rvsflag; // revers
byte csrflag;

dbmInfo **infoBlocks= (dbmInfo **)0x518; // loaded dbm file info blocks

void scrollUp();

void cg_init() {
    mega65_io_enable();
    infoBlockCount= 0;
    for (cgi= 0; cgi < MAX_DBM_BLOCKS; ++cgi) {
        infoBlocks[cgi]= NULL;
    }
    cg_freeGraphAreas();
    bgcolor(COLOR_BLACK);
    bordercolor(COLOR_BLACK);
    cg_go16bit(0, 0);
    cg_loadDBM("borders.dbm", 0x13000, SYSPAL);
    cg_resetPalette(); // assumes standard colours at 0x13800
    cg_textcolor(COLOR_GREEN);
    cg_clrscr();
    gPal= 0;
    rvsflag= 0;
    gPalDir= 1;
}

void cg_pushWin() {
    saveWin[++winCount]= currentWin;
    cg_resetwin();
}

void cg_popWin() {
    currentWin= saveWin[winCount--];
    cg_gotoxy(currentWin.xc, currentWin.yc);
}

void cg_clearBottomLine() {
    cg_resetwin();
    cg_block_raw(0, gScreenRows - 1, gScreenColumns - 1, gScreenRows - 1, 32,
                 0);
    cg_gotoxy(0, gScreenRows - 1);
}
void cg_revers(byte r) { rvsflag= r; }

void cg_resetPalette() {
    mega65_io_enable();
    cg_loadPalette(SYSPAL, 255, false);
}

void cg_fatal(const char *format, ...) {
    char buf[160];
    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    cg_go8bit();
    bordercolor(2);
    textcolor(2);
    bgcolor(0);
    puts("## dr fatal error ##");
    puts(buf);
    while (1)
        ;
}

void cg_freeGraphAreas(void) {
    for (cgi= 0; cgi < infoBlockCount; ++cgi) {
        if (infoBlocks[cgi] != NULL) {
            free(infoBlocks[cgi]);
            infoBlocks[cgi]= NULL;
        }
    }
    nextFreeGraphMem= GRAPHBASE;
    nextFreePalMem= PALBASE;
    infoBlockCount= 0;
}

/*
very simple graphics memory allocation scheme:
try to find space in 128K beginning at GRAPHBASE, without
crossing bank boundaries. If everything's full, bail out.
*/

himemPtr cg_allocGraphMem(word size) {
    himemPtr adr= nextFreeGraphMem;
    if (nextFreeGraphMem + size < GRAPHBASE + 0x10000) {
        nextFreeGraphMem+= size;
        return adr;
    }
    if (nextFreeGraphMem < GRAPHBASE + 0x10000) {
        nextFreeGraphMem= GRAPHBASE + 0x10000;
        adr= nextFreeGraphMem;
    }
    if (nextFreeGraphMem + size < GRAPHBASE + 0x20000) {
        nextFreeGraphMem+= size;
        return adr;
    }
    return NULL;
}

himemPtr cg_allocPalMem(word size) {
    himemPtr adr= nextFreePalMem;
    if (nextFreePalMem < 0x18000) {
        nextFreePalMem+= size;
        return adr;
    }
    return NULL;
}

char asciiToPetscii(byte c) {
    if (c >= 65 && c <= 95) {
        return c - 64;
    }
    if (c >= 96 && c < 192) {
        return c - 32;
    }
    if (c >= 192) {
        return c - 128;
    }
    return c;
}

void cg_go16bit(byte h640, byte v400) {
    mega65_io_enable();
    POKE(53297L, 96); // quit bitplane mode

    VIC4CTRL|= 0x04; // enable full colour for characters with high byte set
    VIC4CTRL|= 0x01; // enable 16 bit characters

    if (h640) {
        VIC3CTRL|= 0x80; // enable H640
        gScreenColumns= 80;
    } else {
        VIC3CTRL&= 0x7f; // disable H640
        gScreenColumns= 40;
    }

    if (v400) {
        VIC3CTRL|= 0x08;
        gScreenRows= 27 * 2;
    } else {
        gScreenRows= 27;
        VIC3CTRL&= 0xf7;
    }

    gScreenSize= gScreenRows * gScreenColumns;
    lfill_skip(SCREENBASE, 32, gScreenSize, 2);
    lfill(COLBASE, 0, gScreenSize * 2);

    HOTREG&= 127;     // disable hotreg
    POKE(53320u, 72); // top border position
    POKE(53326u, 72); // top text position
    POKE(53371u, gScreenRows);

    // move colour RAM because of stupid C65 ROM himem usage
    POKE(53348u, COLOUR_RAM_OFFSET & 0xff);
    POKE(53349u, (COLOUR_RAM_OFFSET >> 8) & 0xff);

    CHRCOUNT= gScreenColumns;
    LINESTEP_LO= gScreenColumns * 2;
    LINESTEP_HI= 0;

    SCNPTR_0= SCREENBASE & 0xff; // screen to 0x12000
    SCNPTR_1= (SCREENBASE >> 8) & 0xff;
    SCNPTR_2= (SCREENBASE >> 16) & 0xff;
    SCNPTR_3&= 0xF0 | ((SCREENBASE) << 24 & 0xff);

    currentWin.xc= 0;
    currentWin.yc= 0;
    textcolor16= 5;
    currentWin.x0= 0;
    currentWin.x1= gScreenColumns - 1;
    currentWin.y0= 0;
    currentWin.y1= gScreenRows - 1;
    currentWin.width= gScreenColumns;
    currentWin.height= gScreenRows;
}

void cg_go8bit() {
    mega65_io_enable();
    POKE(53297L, 96); // quit bitplane mode
    SCNPTR_0= 0x00;   // screen back to 0x800
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
    cg_resetPalette();
}

void cg_plotExtChar(byte x, byte y, byte c) {
    word charIdx;
    long adr;
    charIdx= (EXTCHARBASE / 64) + c;
    adr= SCREENBASE + (x * 2) + (y * gScreenColumns * 2);
    lpoke(adr, charIdx % 256);
    lpoke(adr + 1, charIdx / 256);
}

void cg_addGraphicsRect(byte x0, byte y0, byte width, byte height,
                        himemPtr bitmapData) {
    static byte x, y;
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

/**
 * @brief allocate memory for DBM file and load it
 *
 * @param filename name of DBM file to load
 * @param address address to load bitmap (or 0 for automatic allocation)
 * @param pAddress address to load palette (or 0 for automatic allocation)
 * @return dbmInfo* info block containging start address and metadata
 */

dbmInfo *cg_loadDBM(char *filename, himemPtr address, himemPtr paletteAddress) {

    static byte numColumns, numRows, numColours;
    static byte dbmOptions;
    static byte reservedSysPalette;

    FILE *dbmfile;
    byte *palette;
    word palsize;
    word imgsize;
    word bytesRead;
    himemPtr bitmampAdr;
    himemPtr palAdr;
    dbmInfo *info;

    info= NULL;

    if (!address) {
        info= (dbmInfo *)malloc(sizeof(dbmInfo));
        infoBlocks[infoBlockCount++]= info;
    }

    dbmfile= fopen(filename, "rb");

    if (!dbmfile) {
        cg_fatal("dbmnf %s", filename);
    }
    fread(drbuf, 1, 9, dbmfile);
    /*
        if (0 != memcmp(drbuf, "dbmp", 4)) {
            cg_fatal("ndbm %s", filename);
        }
        */
    numRows= drbuf[5];
    numColumns= drbuf[6];
    dbmOptions= drbuf[7];
    numColours= drbuf[8];
    reservedSysPalette= dbmOptions & 2;

    palsize= numColours * 3;
    palette= (byte *)malloc(palsize);
    fread(palette, 3, numColours, dbmfile);

    mega65_io_enable();

    if (!paletteAddress) {
        palAdr= cg_allocPalMem(palsize);
        if (palAdr == NULL) {
            cg_fatal("nopal");
        }
    } else {
        palAdr= paletteAddress;
    }
    lcopy((long)palette, palAdr, palsize);
    free(palette);
    imgsize= numColumns * numRows * 64;

    fread(drbuf, 1, 3, dbmfile);
    if (0 != memcmp(drbuf, "img", 3)) {
        cg_fatal("noimg %s", filename);
    }

    if (!address) {
        bitmampAdr= cg_allocGraphMem(imgsize);
        if (bitmampAdr == NULL) {
            cg_fatal("nomem %s", filename);
        }
    } else {
        bitmampAdr= address;
    }

    bytesRead= readExt(dbmfile, bitmampAdr);
    fclose(dbmfile);
    mega65_io_enable();

    if (info != NULL) {
        info->columns= numColumns;
        info->rows= numRows;
        info->size= bytesRead;
        info->baseAdr= bitmampAdr;
        info->paletteAdr= palAdr;
        info->paletteSize= numColours;
        info->reservedSysPalette= reservedSysPalette;
    }

    return info;
}

void cg_loadPalette(himemPtr adr, byte size, byte reservedSysPalette) {

    himemPtr cgi;
    static byte r, g, b;
    himemPtr colAdr;

    for (cgi= 0; cgi < size; ++cgi) {
        if (reservedSysPalette && (cgi <= 15)) {
            continue;
        }
        colAdr= cgi * 3;
        r= lpeek(adr + colAdr);     //  palette[colAdr];
        g= lpeek(adr + colAdr + 1); // palette[colAdr + 1];
        b= lpeek(adr + colAdr + 2); // palette[colAdr + 2];
        POKE(0xd100u + cgi, r);
        POKE(0xd200u + cgi, g);
        POKE(0xd300u + cgi, b);
    }
}

void cg_displayDBMInfo(dbmInfo *info, byte x0, byte y0) {
    mega65_io_enable();
    cg_loadPalette(info->paletteAdr, info->paletteSize,
                   info->reservedSysPalette);
    cg_addGraphicsRect(x0, y0, info->columns, info->rows, info->baseAdr);
}

/**
 * @brief load DBM file and display it
 *
 * @param filename DBM file to load
 * @param x0 origin x
 * @param y0 origin y
 * @return dbmInfo* associated dbmInfo block for file
 */

dbmInfo *cg_displayDBMFile(char *filename, byte x0, byte y0) {
    dbmInfo *info;
    info= cg_loadDBM(filename, NULL, NULL);
    cg_displayDBMInfo(info, x0, y0);
    return info;
}

void scrollUp() {
    static byte y;
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
    currentWin.xc= 0;
    currentWin.yc++;
    if (currentWin.yc > currentWin.height) {
        currentWin.yc= currentWin.height;
        scrollUp();
    }
}

void __putc(byte x, byte y, byte c, byte exAttr) {
    word adrOffset;
    adrOffset= (x * 2) + (y * 2 * gScreenColumns);
    lpoke(SCREENBASE + adrOffset, c);
    lpoke(SCREENBASE + adrOffset + 1, 0);
    lpoke(COLBASE + adrOffset + 1, textcolor16 | exAttr);
    lpoke(COLBASE + adrOffset, 0);
}

byte cg_wherex() { return currentWin.xc; }

byte cg_wherey() { return currentWin.yc; }

void cg_putc(char c) {
    static char out;
    if (!c) {
        return;
    }
    if (c == '\n') {
        cr();
        return;
    }
    if (currentWin.xc >= currentWin.width) {
        return;
    }
    out= asciiToPetscii(c);
    if (rvsflag) {
        out|= 128;
    }
    __putc(currentWin.xc + currentWin.x0, currentWin.yc + currentWin.y0, out,
           0);
    currentWin.xc++;
/*
    if (currentWin.xc >= currentWin.width) {
        currentWin.yc++;
        currentWin.xc= 0;
        if (currentWin.yc > currentWin.height) {
            currentWin.yc= currentWin.height;
            scrollUp();
        }
    }
    */

    if (csrflag) {
        __putc(currentWin.xc + currentWin.x0, currentWin.yc + currentWin.y0,
               CURSOR_CHARACTER, 16);
    }
}

void _debug_cg_puts(const char *s) {
    const char *current= s;
    while (*current) {
        cg_putc(*current++);
    }
}

void cg_puts(const char *s) {
#ifdef DEBUG
    char out[16];
#endif
    const char *current= s;
    while (*current) {
        cg_putc(*current++);
    }
#ifdef DEBUG
    cg_pushWin();
    currentWin.x0= 0;
    currentWin.y0= 0;
    currentWin.xc= 36;
    currentWin.yc= 0;
    sprintf(out, "%x", _heapmaxavail());
    _debug_cg_puts(out);
    cg_popWin();
#endif
}

void cg_putsxy(byte x, byte y, char *s) {
    cg_gotoxy(x, y);
    cg_puts(s);
}

void cg_putcxy(byte x, byte y, char c) {
    cg_gotoxy(x, y);
    cg_putc(c);
}

void cg_cursor(byte onoff) {
    csrflag= onoff;
    __putc(currentWin.xc + currentWin.x0, currentWin.yc + currentWin.y0,
           (csrflag ? CURSOR_CHARACTER : 32), (csrflag ? 16 : 0));
}

void box16(byte x0, byte y0, byte x1, byte y1, byte b, byte c) {
    static byte x, y;
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
    currentWin.xc= x;
    currentWin.yc= y;
}

int cg_printf(const char *format, ...) {
    int ret;
    char buf[160];
    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    cg_puts(buf);
    return ret;
}

void cg_clrscr() {
    cg_block_raw(currentWin.x0, currentWin.y0, currentWin.x1, currentWin.y1, 32,
                 textcolor16);
    cg_gotoxy(0, 0);
}

void cg_resetwin() { cg_setwin(0, 0, gScreenColumns, gScreenRows); }

void cg_setwin(byte x0, byte y0, byte width, byte height) {
    currentWin.x0= x0;
    currentWin.y0= y0;
    currentWin.x1= x0 + width - 1;
    currentWin.y1= y0 + height - 1;
    currentWin.width= width;
    currentWin.height= height;
    currentWin.xc= 0;
    currentWin.yc= 0;
    cg_gotoxy(0, 0);
}

void cg_emptyBuffer(void) {
    while (kbhit()) {
        cgetc();
    }
}

unsigned char cg_cgetc(void) {
    unsigned char k;
    do {
        k= PEEK(0xd610u);
    } while (k == 0);
    POKE(0xD610U, 0);
    return k;
}

char cg_getkey(void) {
    cg_emptyBuffer();
    return cgetc();
}

char *cg_input(byte maxlen) {
    static byte len, ct;
    char current;
    char *ret;
    len= 0;
    ct= csrflag;
    cg_cursor(1);
    do {
        current= cgetc();
        if (current != '\n') {
            if (current >= 32) {
                if (len < maxlen) {
                    drbuf[len]= current;
                    drbuf[len + 1]= 0;
                    cg_putc(current);
                    ++len;
                }
            } else if (current == 20) {
                // del pressed
                if (len > 0) {
                    cg_cursor(0);
                    cg_gotoxy(currentWin.xc - 1, currentWin.yc);
                    cg_putc(' ');
                    cg_gotoxy(currentWin.xc - 1, currentWin.yc);
                    cg_cursor(1);
                    --len;
                }
            }
        }
    } while (current != '\n');
    len++;
    ret= (char *)malloc(len);
    strncpy(ret, drbuf, len);
    cg_cursor(ct);
    return ret;
}

void cg_clearFromTo(byte start, byte end) {
    cg_block_raw(0, start, 40, end, 32, textcolor16);
}

void cg_clearLower(byte num) { cg_clearFromTo(gScreenRows - num, gScreenRows); }

void cg_line(byte y, byte x0, byte x1, byte character, byte col) {
    word bas;
    byte width;

    width= x1 - x0 + 1;
    bas= x0 * 2 + (y * gScreenColumns * 2);

    // use DMAgic to fill FCM screens... PGS, I love you!
    lfill_skip(SCREENBASE + bas, character, width, 2);
    lfill_skip(SCREENBASE + bas + 1, 0, width, 2);
    lfill_skip(COLBASE + bas, 0, width, 2);
    lfill_skip(COLBASE + bas + 1, col, width, 2);

    return;
}

void cg_block_raw(byte x0, byte y0, byte x1, byte y1, byte character,
                  byte col) {
    static byte i;
    for (i= y0; i <= y1; ++i) {
        cg_line(i, x0, x1, character, col);
    }
}

void cg_center(byte x, byte y, byte width, char *text) {
    static byte l;
    l= strlen(text);
    if (l >= width - 2) {
        cg_gotoxy(x, y);
        cg_puts(text);
        return;
    }
    cg_gotoxy(-1 + x + width / 2 - l / 2, y);
    cg_plotExtChar(
        currentWin.xc - 1, currentWin.yc,
        H_COLUMN_END); // leave space for imprint effect (thx tayger!)
    cg_puts(text);
    cg_plotExtChar(currentWin.xc, currentWin.yc, H_COLUMN_START);
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

void cg_bordercolor(unsigned char c) { POKE(VIC_BASE + 0x20, c); }

void cg_bgcolor(unsigned char c) { POKE(VIC_BASE + 0x21, c); }

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

void cg_hlinexy(byte x0, byte y, byte x1, byte secondary) {
    static byte lineChar;
    lineChar= secondary ? 6 : 0;
    for (cgi= x0; cgi <= x1; cgi++) {
        cg_plotExtChar(cgi, y, lineChar);
    }
}

void cg_vlinexy(byte x, byte y0, byte y1) {
    cg_plotExtChar(x, y0, 2);
    cg_plotExtChar(x, y1, 3);
    for (cgi= y0 + 1; cgi <= y1 - 1; cgi++) {
        cg_plotExtChar(x, cgi, 1);
    }
}

void cg_frame(byte x0, byte y0, byte x1, byte y1) {
    cg_hlinexy(x0, y0, x1, 0);
    cg_hlinexy(x0, y1, x1, 0);
    cg_vlinexy(x0, y0 + 1, y1 - 1);
    cg_vlinexy(x1, y0 + 1, y1 - 1);
}

void cg_borders(byte showSubwin) {
    cg_frame(0, 0, 39, 25);
    if (showSubwin) {
        cg_vlinexy(16, 1, 15);
        cg_vlinexy(0, 1, 15);
        cg_hlinexy(0, 16, 39, 0);
    }
}

void cg_titlec(byte tcol, byte splitScreen, char *t) {
    static byte splitPos= 12;

    cg_clrscr();
    cg_borders(false);
    if (splitScreen) {
        if (splitScreen != true) {
            splitPos= splitScreen;
        }
        cg_hlinexy(1, splitPos, 38, 1);
    }
    cg_textcolor(tcol);
    cg_center(0, 0, 40, t);
    cg_gotoxy(0, 3);
}
