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
	char title[40] = "Welcome to ";
	strcat(title,gCities[cityIdx]);
	cg_titlec(6,5,title);
	exit(0);
}

void initEngine(void)
{
	cg_init();
	puts("ARCHAIC(tm) engine v0.1");
	puts("Written by Stephan Kleinert");
	puts("Seven Turtles Software, 2019");
	initGuild();
}

int main()
{
	char choice;
	initEngine();
	clrscr();
	cg_borders();
	cputsxy(2, 12, "1 - load saved game");
	cputsxy(2, 13, "2 - start in ");
	cputs(gCities[0]);
	do
	{
		choice = cgetc();
	} while (choice != '1' && choice != '2');
	if (choice == '1')
	{
		loadSaved();
	}

	runCity(currentCity);

	return 0;
}