#include "config.h"
#include "types.h"

// memory allocation

#define SCREENBASE 0x12000l     // 16 bit screen
#define COLBASE 0xff80800l      // colours
#define GRAPHBASE 0x40000l      // bitmap characters
#define EXTCHARBASE 0x13000l    // extended characters for map
#define SYSPAL 0x14000          // system palette
#define PALBASE 0x14300l        // palettes for loaded images
#define SEENMAP_BASE 0x18000l   // seen map
#define ITEM_BASE 0x8020000     // item table
#define MONSTERS_BASE 0x8021000 // monster table

#define DRBUFSIZE 0xff

/*  ----
    main
    ---- */

extern char *drbuf; // general purpose buffer
extern byte devmode; // devmode flag

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

extern character **party; // current adventuring party
extern long int gPartyGold;         // gold to be distributed when entering city
extern long int gPartyExperience;   // xp to be distributed when entering city

/*  -------
    dungeon
    ------- */

extern himemPtr seenMap;  // map of seen spaces in dungeon
