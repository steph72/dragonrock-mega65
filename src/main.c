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
#include "guild.h"
#include "config.h"

unsigned char currentCity;
unsigned char outbuf[80]; 

void initEngine(void);
void runCityMenu(void);
void runGuildMenu(void);
void loadSaved(void);

void loadSaved(void)
{
	puts("\n\nloadSaved not implemented yet");
	exit(0);
}

void runGuildMenu(void)
{
	unsigned char cmd = 0;
	unsigned char quitGuild = 0;
	sprintf(outbuf,"%s Guild",gCities[currentCity]);

	while (!quitGuild)
	{
		cg_titlec(8, 5, 1, outbuf);
		cputsxy(2, 14, "L)ist guild members       T)raining");
		cputsxy(2, 15, "N)ew guild member         S)pells");
		cputsxy(2, 16, "P)urge guild member");
		cputsxy(2, 17, "A)dd to party");
		cputsxy(2, 18, "D)rop from party");
		cputsxy(2, 19, "eX)it guild");
		cputsxy(2, 21, "Command:");
		cursor(1);
		do
		{
			cmd = cgetc();
		} while (strchr("lnpadxts", cmd) == NULL);
		cursor(0);
		
		switch (cmd)
		{
		case 'n':
			newGuildMember();
			break;

		case 'l':
			listGuildMembers();
			break;

		case 'x':
			return;
			break;

		default:
			break;
		}
	}
}

void runCityMenu(void)
{
	unsigned char cmd = 0;
	unsigned char quitCity = 0;

	while (!quitCity)
	{
		sprintf(outbuf,"%s (%d)",gCities[currentCity],currentCity+1);
		cg_titlec(6, 5, 1, outbuf);
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

		cursor(0);

		switch (cmd)
		{
		case 'l':
			quitCity = 1;
			break;

		case 'g':
			runGuildMenu();
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
	} while (strchr("12", choice) == NULL);

	if (choice == '1')
	{
		loadSaved();
	}
	else
	{
		currentCity = 0;
	}

	runCityMenu();

	return 0;
}