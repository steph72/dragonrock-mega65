
#include "types.h"

#ifndef __CONGUI
#define __CONGUI

#define SCREENBASE 0x12000l
#define COLBASE 0xff80000l
#define GRAPHBASE 0x40000l

// --------------- graphics ------------------

typedef struct _dbmInfo {
    himemPtr baseAdr;
    byte columns;
    byte rows;
    word size;
} dbmInfo;

typedef struct _shapeInfo {
    byte shapeID;
    dbmInfo *shape0;
    dbmInfo *shape1;
} shapeInfo;

typedef struct _textwin {
    byte x0;
    byte y0;
    byte x1;
    byte y1;
    byte width;
    byte height;
} textwin;

void cg_init(void);
void cg_fatal(const char *format, ...);

void cg_titlec(byte lcol, byte tcol, byte splitScreen, char *t);
void cg_borders(void);
void cg_emptyBuffer(void);
char cg_getkey(void);
char cg_getkeyP(byte x, byte y, const char *prompt);

void cg_clearLower(byte num);
void cg_clearFromTo(byte start, byte end);
void cg_clearGraphics(void);
void cg_clearText(void);
void cg_clear(void);

void cg_stepColor(void);
void cg_stopColor(void);

void cg_block_raw(byte x0, byte y0, byte x1, byte y1, byte character, byte col);
void cg_colorLine(byte y, byte x0, byte x1, byte col);
void cg_line(byte y, byte x0, byte x1, byte character, byte col);

void cg_setPalette(byte num, byte red, byte green, byte blue);
void cg_resetPalette();

void cg_go16bit(byte h640, byte v400);
void cg_go8bit();
void cg_clrscr();
void cg_putc(char c);
void cg_puts(char *s);
void cg_putsxy(byte x, byte y, char *s);
void cg_putcxy(byte x, byte y, char c);
int cg_printf(const char *format, ...);
void cg_gotoxy(byte x, byte y);
void cg_textcolor(byte c);
void cg_setwin(byte x0, byte y0, byte width, byte height);

void cg_freeGraphAreas(void);
void cg_addGraphicsRect(byte x0,byte y0, byte width, byte height, himemPtr bitmapData);
dbmInfo *cg_loadDBM(char *filename);
void cg_displayDBMInfo(dbmInfo *info, byte x0, byte y0);
dbmInfo *cg_displayDBMFile(char *filename, byte x0, byte y0);

void cg_test();

#endif