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

#define true 1
#define false 0

typedef unsigned char byte;
typedef unsigned int word;

typedef byte attrT;
typedef byte itemT;
typedef unsigned long himemPtr;

typedef enum attribute { aSTR, aINT, aWIS, aDEX, aCON, aCHR } attrKind;

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

typedef enum _raceT { rt_human, rt_dwarf, rt_elf, rt_halfelf, rt_gnome } raceT;

typedef enum _encCommand {
    ec_nothing =0,
    ec_thrust,
    ec_attack,
    ec_slash,
    ec_lunge,
    ec_spell,
    ec_parry,
} encCommand;

typedef enum _pcr {
    preCombatResultGreet,
    preCombatResultSurrender,
    preCombatResultMercy,
    preCombatResultFleeFailure,
    preCombatResultFleeSuccess,
    preCombatResultBeginFight,
    preCombatResultNoResponse
} preCombatResult;

typedef enum _encResult {
    encUndef= 0,  // no encounter
    encWon,       // the party wins
    encSurrender, // the monsters surrender
    encGreet,     // both partys greet
    encFight,     // encounter not over; monsters want to fight
    encMercy,     // monsters show mercy
    encFled,      // the party has fled
    encDead       // the party is dead
} encResult;

typedef enum _cstateType {
    deleted= 0,
    down= 1,
    asleep= 2,
    dead= 3,
    awake= 4,
    charmed= 5,
    surrendered= 6
} characterStateT;

typedef enum _itemType {
    it_armor,
    it_shield,
    it_weapon,
    it_missile,
    it_potion,
    it_scroll,
    it_special
} itemType;

typedef enum _monstertype {
    mt_animal= 0x01,
    mt_humanoid= 0x02,
    mt_magical= 0x04,
    mt_unique= 0x08
} monsterType;

typedef enum _attackType {
    at_fists= 0x01,
    at_weapon= 0x02,
    at_fire= 0x04,
    at_ice= 0x08,
    at_claws= 0x10,
    at_drain= 0x20,
    at_breath= 0x40,
    at_spell= 0x80
} attackType;

typedef enum _spellClass {
    sc_priest= 0x01,
    sc_necromancer= 0x02,
    sc_battlemage= 0x04
} spellClassT;

typedef struct _cityCoords {
    byte mapNr;
    byte x;
    byte y;
} cityCoordsT;

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

typedef struct _item { // inventory item
    unsigned int id;
    unsigned int namePtr;
    itemType type;
    byte val1; // armor, weapon: minStrength needed; scroll: scrollID
    byte val2; // armor: ac value; weapon: hit dice
    byte val3; // weapon, armor, ring: bonus
    unsigned int price;
} item;

typedef struct _ditem { // dungeon item
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
    byte param7;
} opcode;

typedef struct _monsterDef {
    unsigned int id;             // 0-1
    byte level;                  // 2
    byte spriteID;               // 3
    monsterType type;            // 4
    unsigned int namePtr;        // 5-6
    unsigned int pluralnamePtr;  // 7-8
    signed char armorClass;      // 9
    attackType aType[4];         // 10-13
    byte minDmg[4];              // 14-17
    byte maxDmg[4];              // 18-21
    signed char hitModifier[4];  // 22-25
    byte hpPerLevel;             // 26
    byte mpPerLevel;             // 27
    signed char courageModifier; // 28
    spellClassT spellClass;      // 29
    int xpBaseValue;             // 30-31
} monsterDef;

typedef struct monster {
    unsigned int monsterDefID;
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
    byte guildSlot;
    byte spriteID;
    byte city;
    byte level;
    int age;
    char name[16];
    raceT aRace;
    classT aClass;
    attrT attributes[NUM_ATTRS];
    byte spellMap[8];
    int aMaxHP;
    int aMaxMP;
    int aHP;
    int aMP;
    int gold;
    int xp;
    itemT inventory[INV_SIZE];
    itemT weapon;
    itemT shield;
    itemT armor;
    // encounter attributes
    signed char initiative;
    encCommand currentEncounterCommand;
    byte encSpell;
    byte encDestination;
} character;

typedef struct {
    byte x1, y1, x2, y2;
    unsigned int opcodeIndex;
} daemonEntry;

typedef struct _dungeonDescriptorS {
    himemPtr test;
    himemPtr mapdata;    // external pointer to mapdata
    himemPtr dungeon;    // external pointer to dungeon map
    himemPtr opcodesAdr; // external pointer to opcode list
    himemPtr *feelTbl;   // pointer to message list (list of external pointers)
    daemonEntry *daemonTbl; // pointer to daemons table
    int numDaemons;         // number of entries in daemons lookup table
    int numCoords;          // number of entries in coords lookup table
    byte dungeonMapWidth;
    byte dungeonMapHeight;
} dungeonDescriptor;

typedef struct _hresult { // hit result
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