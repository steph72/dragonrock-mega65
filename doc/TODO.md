# TODO

cc65 V2.18 - Git de630a12

## misc
make game config universal & live in himem
DONE failsafe center
DONE center in title
DONE use DMA for screen fills
DONE refactor dungeon text output area concept
DONE fix input routine for empty input

## city
- DONE implement "you must assemble a party first"
- implement armory
- implement inn
- implement mystic
- implement guild sync
- finish guild


## dungeon
- DONE sanitize filenames of maps and dungeons 
- implement persistance
- fix reload of current outdoor map
- DONE put seenmap into attic

## dungeonscript
- modify ADDENC with min/max
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

