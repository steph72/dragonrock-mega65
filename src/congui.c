/*
 * congui.c
 *
 * Copyright (C) 2019 - Stephan Kleinert
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <conio.h>
#include <cbm.h>
#include <string.h>
#include "types.h"

void cg_init()
{
	bgcolor(0);
	bordercolor(0);
	textcolor(5);
	cbm_k_bsout(11);  // disable shift+cmd
	cbm_k_bsout(14); // lowercase charset
	clrscr();
}

void cg_borders(void)
{
	chlinexy(0, 0, 40);
	chlinexy(0, 24, 40);
}

void cg_titlec(byte lcol, byte tcol, char *t)
{
	clrscr();
	textcolor(lcol);
	cg_borders();
	chlinexy(0,2,40);
	textcolor(tcol);
	cputsxy(20-(strlen(t)/2), 1, t);
	gotoxy(0, 3);
}
