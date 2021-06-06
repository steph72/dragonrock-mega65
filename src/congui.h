
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


void cg_setPalette(byte num, byte red, byte green, byte blue);
