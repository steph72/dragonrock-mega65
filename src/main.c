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


void initEngine(void);

void initEngine(void)
{
	cg_init();
	cg_println("ARCHAIC(tm) engine v0.1");
	cg_println("Written by Stephan Kleinert");
	cg_println("Seven Turtles Software, 2019");
	initGuild();
}

int main()
{
	initEngine();
	return 0;
}