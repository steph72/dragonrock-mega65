#ifndef __menuH
#define __menuH

#include "congui.h"
#include "types.h"

#define runBottomMenu(A) runMenu(A, 0, 24, 0, 0)
#define runBottomMenuN(A) runMenu(A, 0, 24, 0, 1)

byte runMenu(char *entries[], byte x, byte y, byte vertical,
             byte enableNumberShortcuts);

#endif