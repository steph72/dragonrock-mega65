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

#define SCREEN ((unsigned char *)0xc00)
#define BUFSIZE 0xb0

extern char* drbuf;
typedef unsigned char byte;

typedef byte  raceT;
typedef byte  classT;
typedef byte  attrT;
typedef byte  itemT;

typedef enum _encCommand {
	ec_thrust,
	ec_attack,
	ec_slash,
	ec_parry,
	ec_magic
} encCommand;

typedef enum _encResult {
	encWon, 	// the party wins
	encSurrender, // the monsters surrender
	encGreet,	// both partys greet
	encFight, 	// encounter not over; monsters want to fight
	encMercy,	// monsters show mercy
	encFled, 	// the party has fled
	encDead		// the party is dead
} encResult;


typedef enum _cstateType {
	deleted=0, 
	dead=1, 
	alive=2
} characterStateT;

typedef enum _itemType {
	it_armor, it_shield, it_weapon, it_potion, it_scroll, it_special 
} itemType;

typedef enum _gameMode {
	gm_city, gm_merchant, gm_outdoor, gm_dungeon, gm_encounter
} gameMode;

typedef enum _monstertype {
	mt_animal   = 0x01,
	mt_humanoid = 0x02, 
	mt_magical  = 0x04,
	mt_unique   = 0x08
} monsterType;

typedef enum _attackType {
	at_fists   = 0x01,
	at_weapon  = 0x02,
	at_claws   = 0x04,
	at_spell   = 0x08,
	at_breath  = 0x10,
	at_wtouch  = 0x20,
	at_ice     = 0x40,
	at_fire    = 0x80
} attackType;

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
	byte nextOpcodeIndex;
} opcode;

typedef struct _monsterDef {
	byte id;
	byte level;
	byte spriteID;
	monsterType type;
	char *name;
	char *pluralname;	// nil if 's' suffix suffices :) 
	signed char armorClass;
	byte hitDice;
	byte hpPerLevel;
	byte mpPerLevel;
	byte numAttacks;
	byte courageModifier;
	signed char hitModifier;
	attackType aType;
	int xpBaseValue;
} monsterDef;

typedef struct monster {
	monsterDef *def;
	byte level;
	signed char initiative;
	int hp;
	int mp;
} monster;

typedef struct _character {
	characterStateT status;
	byte	guildSlot;
	byte    spriteID;
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
	signed char    initiative;
} character;

typedef struct _dungeonDescriptorS {
    dungeonItem *dungeon;    // pointer to dungeon map
    opcode *opcodesAdr;      // pointer to opcode list
    unsigned char **feelTbl; // pointer to message list
    byte dungeonMapWidth;
    byte dungeonMapHeight;
    byte startX;
    byte startY;
} dungeonDescriptor;

#endif