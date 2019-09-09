

#include <cbm.h>
#include <conio.h>
#include <plus4.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "congui.h"
#include "guild.h"
#include "guildLoader.h"
#include "types.h"

#include "charset.h"

character *party[PARTYSIZE];

byte loadParty(void) {
    static FILE *infile;
    static byte i, count;
    character *newChar;

    infile= fopen("pdata", "r");
    if (!infile) {
        return false;
    }

    count = fgetc(infile);

    for (i=0;i<count;++i) {
        printf(".");
        newChar = malloc(sizeof(character));
        fread(newChar,sizeof(character),1,infile);
        party[i]=newChar;
    }

    fclose(infile);
    return true;
}

