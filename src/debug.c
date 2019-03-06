#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include "congui.h"

char dbgOut[16];

void dbgMem(unsigned int size, void *addr, char *type)
{
	itoa(size, dbgOut, 16);
	cputs("$");
	cputs(dbgOut);
	cputs(" ");
	cputs(type);
	cputs(" bytes at $");
	itoa((unsigned int)addr, dbgOut, 16);
	puts(dbgOut);
}
