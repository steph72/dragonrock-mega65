#include "config.h"
#include "types.h"

#ifndef DR_GLOBALS
#define DR_GLOBALS

// memory allocation

#define SCREENBASE 0x12000l            // 16 bit screen
#define EXTCHARBASE 0x13000l           // extended characters for map
#define SYSPAL 0x14000                 // system palette
#define PALBASE 0x14300l               // palettes for loaded images
#define CFG_STORAGE_BASE 0x16000l      // base address for high memory storage
#define SEENMAP_BASE 0x18000l          // seen map
#define GRAPHBASE 0x40000l             // bitmap characters
#define ATTIC_DUNGEON_DATA 0x8000000   // dungeon data (64K)
#define ATTIC_DUNGEON_CODE 0x8010000   // dungeon code overlay (16K, copied when needed)
#define ATTIC_CITY_CODE 0x8014000      // city code (16K)
#define ATTIC_ENCOUNTER_CODE 0x8018000 // encounter code (16K)

#define COLBASE 0xff80800l // colours

#define DRBUFSIZE 0xff

/*  ----
    main
    ---- */

extern char *drbuf;  // general purpose buffer
extern byte devmode; // devmode flag

extern himemPtr itemBase;
extern himemPtr monstersBase;
extern himemPtr citiesBase;

/*  ----------
    dispatcher
    ---------- */

extern gameModeT gCurrentGameMode; // current game mode

extern byte
    gCurrentDungeonIndex; // current dungeon (==dungeon to enter on mode change)
extern byte gLoadedDungeonIndex; // currently loaded dungeon
extern byte gStartXPos;          // landing position for entering new maps
extern byte gStartYPos;
extern byte gCurrentCityIndex;     // current city index
extern encResult gEncounterResult; // result of last encounter

/*  ---------------
    character/party
    --------------- */

extern character **party;         // current adventuring party
extern long int gPartyGold;       // gold to be distributed when entering city
extern long int gPartyExperience; // xp to be distributed when entering city

/*  -------
    dungeon
    ------- */

extern himemPtr seenMap; // map of seen spaces in dungeon

#endif