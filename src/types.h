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


typedef unsigned char byte;

typedef byte  raceT;
typedef byte  classT;
typedef byte  attrT;

typedef enum _cstateType {
	deleted=0, dead, alive
} characterStateT;

typedef enum _itemType {
	armor, shield, weapon, potion, special 
} itemType;

typedef struct _item {
	char name[16];
	itemType t;
	byte val1;
	byte val2;
	int price;
} item;

typedef struct _character {
	characterStateT status;
	byte    city;
	char	name[16];
	raceT	aRace;
	classT	aClass;
	attrT	aSTR;
	attrT	aINT;
	attrT	aWIS;
	attrT	aDEX;
	attrT	aCON;
	attrT	aCHR;
	int		aMaxHP;
	int		aMaxMP;
	int		aHP;
	int		aMP;
	int		gold;
	item*	inventory[12];
	byte    weapon;
	byte    shield;
	byte    armor;
} character;

#endif