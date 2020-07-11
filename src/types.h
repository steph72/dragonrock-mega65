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

#define SCREEN ((unsigned char *)0x400)
#define BUFSIZE 0xb0
#define TEXTBUF_SIZE 0x400

typedef unsigned char byte;

typedef byte attrT;
typedef byte itemT;

typedef enum attribute {
	aSTR, aINT, aWIS, aDEX, aCON, aCHR
} attrKind;


typedef enum _gameMode {
    gm_init,
    gm_city,
    gm_outdoor,
    gm_dungeon,
    gm_encounter,
    gm_end
} gameModeT;

typedef enum _classT {
	ct_fighter,
	ct_ranger,
	ct_priest,
	ct_wizard,
	ct_thief
} classT;

typedef enum _raceT {
	rt_human,
	rt_dwarf,
	rt_elf,
	rt_halfelf,
	rt_gnome
} raceT;

typedef enum _encCommand {
	ec_nothing,
	ec_thrust,
	ec_attack,
	ec_slash,
	ec_parry,
	ec_magic,
	ec_fireBow
} encCommand;

typedef enum _encResult {
	encUndef=0,	// no encounter 
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
	down=1, 
	asleep=2,
	dead=3, 
	awake=4,
	surrendered=5
} characterStateT;

typedef enum _itemType {
	it_armor, it_shield, it_weapon, it_missile, it_potion, it_scroll, it_special 
} itemType;

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

// --------------- spells ------------------

typedef struct _spell {
	char *name;
	byte spellLevel;
	byte minLevel;
	byte mpNeeded;
	byte minDmg;
	byte maxDmg;
} spell;

// -------------- equipment ----------------

typedef struct _item {			// inventory item
	byte id;
	char *name;
	itemType type;
	byte val1;					// armor, weapon: minStrength needed; scroll: scrollID
	byte val2;					// armor: ac value; weapon: hit dice
	byte val3;					// weapon, armor, ring: bonus
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
	characterStateT status;
	byte hasDoneTurn;
	byte level;
	signed char row;
	signed char column;
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
	byte	spellMap[8];
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
	// encounter attributes
	signed char initiative;
	encCommand	currentEncounterCommand;
	byte		encSpell;
	byte		encDestination;
} character;

typedef long himemPtr;


typedef struct _dungeonDescriptorS {
	himemPtr mapdata;			 // external pointer to mapdata
    himemPtr dungeon;    		 // external pointer to dungeon map
    himemPtr opcodesAdr;      	 // external pointer to opcode list
    himemPtr *feelTbl;           // pointer to message list (list of external pointers)
    byte dungeonMapWidth;
    byte dungeonMapHeight;
    byte startX;
    byte startY;
} dungeonDescriptor;

typedef struct _hresult {		// hit result
	byte success;
	byte critical;
	int hitRoll;
	int hitBonus;
	int damageBonus;
	int acHit;
	int toHit;
	int damage;
	character *theCharacter;
	monster *theMonster;
} hitResult;

#endif