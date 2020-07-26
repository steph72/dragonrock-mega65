
#include "types.h"

void cg_init(void);
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

void cg_center(byte x, byte y, byte width, char *text);
void cg_block(byte x0, byte y0, byte x1, byte y1, byte character, byte col);
void cg_colorLine(byte y, byte x0, byte x1, byte col);
void cg_line(byte y, byte x0, byte x1, byte character, byte col);

byte cg_verticalList(byte x0, byte y0, byte lineSpacing, byte width, byte col,
                     char *items[]);
                          
byte cg_verticalMenu(byte x0, byte y0, byte lineSpacing, byte width, byte menuItemCount);

byte cg_menu(byte width, byte color, char *items[]);
byte cg_horizontalMenu(byte color, byte defaultItem, char *items[]);