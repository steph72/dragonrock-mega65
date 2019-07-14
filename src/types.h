/*
 * types.h
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

#ifndef __drtypes
#define __drtypes

#define NUM_ATTRS 6
#define INV_SIZE 12

#define true   1
#define false  0

typedef unsigned char byte;

typedef byte  raceT;
typedef byte  classT;
typedef byte  attrT;
typedef byte  itemT;

typedef enum _cstateType {
	deleted=0, dead, alive
} characterStateT;

typedef enum _itemType {
	it_armor, it_shield, it_weapon, it_potion, it_scroll, it_special 
} itemType;

typedef struct _item {			// inventory item
	byte id;
	char *name;
	byte type;
	byte val1;
	byte val2;
	int price;
} item;

typedef struct _ditem {			// dungeon item
    byte mapItem;
    byte opcodeID;
} dungeonItem;

typedef struct _opcode {
	byte id;
	byte param1;
	byte param2;
	byte param3;
	byte param4;
	byte param5;
	byte param6;
	byte nextOpcode;
} opcode;

typedef struct _character {
	characterStateT status;
	byte	guildSlot;
	byte    city;
	byte	level;
	int		age;
	char	name[16];
	raceT	aRace;
	classT	aClass;
	attrT   attributes[NUM_ATTRS];
	int		aMaxHP;
	int		aMaxMP;
	int		aHP;
	int		aMP;
	int		gold;
	int	    xp;
	itemT	inventory[INV_SIZE];
	itemT   weapon;
	itemT   shield;
	itemT   armor;
} character;

typedef struct _dungeonDescriptorS {
    dungeonItem *dungeon;    // pointer to dungeon map
    byte *seenSpaces;        // pointer to bit array of seen spaces
    opcode *opcodesAdr;      // pointer to opcode list
    unsigned char **feelTbl; // pointer to message list
    byte dungeonMapWidth;
    byte dungeonMapHeight;
    byte startX;
    byte startY;
} dungeonDescriptor;

#endif