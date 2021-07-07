#ifndef __menuH
#define __menuH

#include "types.h"
#include "congui.h"

#define runBottomMenu(A) runMenu(A,0,24,0)

byte runMenu(char *entries[], byte x, byte y, byte vertical);

#endif