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
void showCurrentParty(void);

void loadSaved(void)
{
	puts("\n\nloadSaved not implemented yet");
	exit(0);
}

void showCurrentParty(void)
{
	static byte i, y;
	static character *c;
	gotoxy(0, 2);
	y = 2;
	for (i = 0; i < PARTYSIZE; i++)
	{
		if (party[i])
		{
			c = party[i];
			++y;
			gotoxy(0, y);
			cprintf("%d %s", i + 1, c->name);
			cputsxy(20, y, gRacesS[c->aRace]);
			cputsxy(24, y, gClassesS[c->aClass]);
			cputsxy(34, y, gStateDesc[c->status]);
		}
	}
}

void runGuildMenu(void)
{
	const char menu[] = "  L)ist new guild members  T)raining\n"
						"  N)ew guild member        S)pells\n"
						"  P)urge guild member\n"
						"  A)dd to party\n"
						"  D)rop from party\n"
						"  eX)it guild\n";

	static unsigned char cmd;
	static unsigned char quitGuild;

	quitGuild = 0;
	sprintf(outbuf, "%s Guild", gCities[currentCity]);

	while (!quitGuild)
	{
		cg_titlec(8, 5, 1, outbuf);
		showCurrentParty();
		gotoxy(0, 14);
		puts(menu);
		cputsxy(2, 22, "Command:");
		cursor(1);
		do
		{
			cmd = cgetc();
		} while (strchr("lnpadxts", cmd) == NULL);
		cursor(0);

		switch (cmd)
		{

		case 'a':
			addToParty();
			break;

		case 'n':
			newGuildMember(currentCity);
			break;

		case 'l':
			listGuildMembers();
			break;

		case 'p':
			purgeGuildMember();
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
	const char menu[] = " Go to\tA)rmory G)uild M)ystic\n"
						"\tI)nn    B)ank  L)eave town\n\n"
						"\tC)ast spell\n"
						"\tU)se item\n"
						"\tS)ave game\n";

	static unsigned char cmd;
	static unsigned char quitCity;

	quitCity = 0;
	while (!quitCity)
	{
		sprintf(outbuf, "%s (%d)", gCities[currentCity], currentCity + 1);
		cg_titlec(6, 5, 1, outbuf);
		showCurrentParty();
		gotoxy(0, 14);
		puts(menu);
		cputsxy(8, 21, "Command:");
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

		case 's':
			saveGuild();
			break;

		default:
			break;
		}
	}
}

void initEngine(void)
{
	const char prompt[] = "ARCHAIC(tm) engine v0.1alpha\n"
					"Written by Stephan Kleinert\n"
					"Copyright (c) 2019 7Turtles Software";
	cg_init();
	puts(prompt);
	initGuild();
}

int main()
{
	static char choice;

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