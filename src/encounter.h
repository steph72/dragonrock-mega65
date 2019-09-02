#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"
#include "config.h"
#include "congui.h"
#include "types.h"
#include "monster.h"

extern monster *gMonsterRow[2][5];

encResult doEncounter(void);