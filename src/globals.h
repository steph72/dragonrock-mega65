#include "config.h"
#include "types.h"

#define SCREEN ((unsigned char *)0x400)
#define ITEM_BASE       0x8020000
#define MONSTERS_BASE   0x8021000
#define DRBUFSIZE       0xff

/*  ----
    main
    ---- */

extern char drbuf[];   // general purpose buffer
extern byte devmode;   // devmode flag

/*  ----------
    dispatcher
    ---------- */

extern gameModeT gCurrentGameMode;  // current game mode

extern byte gCurrentDungeonIndex;   // current dungeon (==dungeon to enter on mode change)   
extern byte gLoadedDungeonIndex;    // currently loaded dungeon
extern byte gStartXPos;           // landing position for entering new maps
extern byte gStartYPos;    
extern byte gCurrentCityIndex;      // current city index
extern encResult gEncounterResult;  // result of last encounter

/*  ---------------
    character/party
    --------------- */

extern character *party[PARTYSIZE]; // current adventuring party
extern long int gPartyGold;         // gold to be distributed when entering city
extern long int gPartyExperience;   // xp to be distributed when entering city

/*  -------
    dungeon
    ------- */

extern byte *seenMap; // map of seen spaces in dungeon
