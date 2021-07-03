
#include "types.h"

#ifndef __CONGUI
#define __CONGUI



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

#define cg_clearxy(x0,y,x1) cg_line(y,x0,x1,32,0)


void cg_init(void);
void cg_fatal(const char *format, ...);

void cg_titlec(byte lcol, byte tcol, byte splitScreen, char *t);
void cg_borders(byte showSubwin);

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
void cg_frame(byte x0, byte y0, byte x1, byte y1);
void cg_hlinexy(byte x0, byte y, byte x1, byte secondary);
void cg_vlinexy(byte x, byte y0, byte y1);

void cg_setPalette(byte num, byte red, byte green, byte blue);
void cg_resetPalette();

void cg_go16bit(byte h640, byte v400);
void cg_go8bit();
void cg_clrscr();
void cg_putc(char c);
void cg_puts(const char *s);
void cg_putsxy(byte x, byte y, char *s);
void cg_putcxy(byte x, byte y, char c);
int cg_printf(const char *format, ...);
void cg_gotoxy(byte x, byte y);
byte cg_wherex();
byte cg_wherey();
void cg_textcolor(byte c);
void cg_setwin(byte x0, byte y0, byte width, byte height);
void cg_line(byte y, byte x0, byte x1, byte character, byte col);
void cg_cursor(byte onoff);
unsigned char cg_cgetc(void);
unsigned char cg_kbhit(void);
void cg_bordercolor(unsigned char c);
void cg_bgcolor(unsigned char c);
void cg_revers(byte r);

void cg_freeGraphAreas(void);
void cg_addGraphicsRect(byte x0,byte y0, byte width, byte height, himemPtr bitmapData);
dbmInfo *cg_loadDBM(char *filename, himemPtr address);
void cg_displayDBMInfo(dbmInfo *info, byte x0, byte y0);
dbmInfo *cg_displayDBMFile(char *filename, byte x0, byte y0);

void cg_plotExtChar(byte x, byte y, byte c);

void cg_test();

#endif