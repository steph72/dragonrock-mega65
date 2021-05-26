# TODO

cc65 V2.18 - Git de630a12

## dungeon
- DONE sanitize filenames of maps and dungeons 
- implement saving the map state
- fix reload of current outdoor map

## dungeonscript
- DONE implement 'random' call and branch
- DONE get rid of start cooords
- DONE implement daemons
- implement globals & global checks
- DONE get rid of "nextOpcIndex" mechanism
- DONE make GOTO a regular dungeon opcode    (param 1/2 == dest)
- DONE make opc&128 a the stop indicator
- DONE get rid of opc&128 as verbose indicator (use param7 instead)

## encounter:
- re-implement whole encounter

## overall
- DONE implement outdoors
- DONE implement user defined shares of xp and coins
- implement armory
- implement inn
