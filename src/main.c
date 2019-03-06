/*
 * Copyright (C) 2019  Stephan Kleinert
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have recxeived a copy of the GNU Lesser General Public
 * License along with main.c; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "types.h"
#include "congui.h"
#include "debug.h"
#include "guild.h"
#include "config.h"

void initEngine(void);

void initEngine(void)
{
	cg_init();
	puts("ARCHAIC(tm) engine v0.1");
	puts("Written by Stephan Kleinert");
	puts("Seven Turtles Software, 2019");
	initGuild();
	puts("Press any key to start");
	cgetc();
}

int main()
{
	char choice;
	initEngine();
	clrscr();
	chlinexy(0, 0, 40);
	chlinexy(0, 24, 40);
	cputsxy(0, 12, "1 - load saved game");
	cputsxy(0, 13, "2 - start in ");
	cputs(gCities[0]);
	do
	{
		choice = cgetc();
	} while (choice != '1' && choice != '2');

	return 0;
}