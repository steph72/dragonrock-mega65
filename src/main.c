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
#include <string.h>

#include "types.h"
#include "congui.h"
#include "debug.h"
#include "guild.h"
#include "config.h"

unsigned char currentCity;

void initEngine(void);
void runCity(unsigned char);
void loadSaved(void);

void loadSaved(void)
{
	puts("loadSaved not implemented yet");
	exit(0);
}

void runCity(unsigned char cityIdx)
{
	unsigned char cmd = 0;
	unsigned char quitCity = 0;
	char title[40] = "";
	char num[5] = " (1)";

	num[2] = '1' + cityIdx;
	strcat(title, gCities[cityIdx]);
	strcat(title, num);

	while (!quitCity)
	{
		cg_titlec(6, 5, title);
		cputsxy(2, 14, "Go to");
		cputsxy(9, 14, "A)rmory  G)uild  M)ystic");
		cputsxy(9, 15, "In)n     B)ank   L)eave town");
		cputsxy(9, 17, "C)ast spell");
		cputsxy(9, 18, "U)se item");
		cputsxy(9, 19, "S)ave game");
		cputsxy(9, 21, "Command:");
		cursor(1);

		do
		{
			cmd = cgetc();
		} while (strchr("agmiblcus", cmd) == NULL);

		switch (cmd)
		{
		case 'l':
			quitCity = 1;
			break;

		default:
			break;
		}
	}
}

void initEngine(void)
{
	cg_init();
	initGuild();
}

int main()
{
	char choice;

	initEngine();
	clrscr();
	cg_borders();
	cputsxy(2, 4, "Welcome to");
	cputsxy(5, 6, "Dragon Rock 1 - The Escape");
	cputsxy(2, 12, "1 - load saved game");
	cputsxy(2, 14, "2 - start in ");
	cputs(gCities[0]);
	do
	{
		choice = cgetc();
	} while (strchr("12",choice)==NULL);
	if (choice == '1')
	{
		loadSaved();
	}

	runCity(currentCity);

	return 0;
}