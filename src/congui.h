
#include "types.h"

void cg_init(void);
void cg_titlec(byte lcol, byte tcol, byte splitScreen, char *t);
void cg_borders(void);
void cg_emptyBuffer(void);
char cg_getkey(void);

void cg_clearLower(byte num);
void cg_clearFromTo(byte start, byte end);
void cg_clearGraphics(void);
void cg_clearText(void);
void cg_clear(void);

void cg_block(byte x0, byte x1, byte y0, byte y1, byte character, byte col);
void cg_line(byte y, byte x0, byte x1, byte character, byte col);
void cg_verticalList(byte x0, byte y0, byte lineSpacing, byte width, byte col, char *items[]);
void cg_verticalMenu(byte x0, byte y0, byte lineSpacing, byte width, byte menuItemCount);




