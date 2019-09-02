

#include <cbm.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <plus4.h>

#include "config.h"
#include "congui.h"
#include "guild.h"
#include "types.h"
#include "guildLoader.h"

character *guild;
character *party[PARTYSIZE];

byte loadGuild(void) {
    static FILE *infile;
    static byte i;
    static byte slot;
    infile= fopen("gdata", "r");
    if (!infile) {
        return false;
    }
    fread(guild, GUILDSIZE * sizeof(character), 1, infile);
    for (i= 0; i < PARTYSIZE; i++) {
        slot= fgetc(infile);
        if (slot != 99) {
            party[i]= &guild[slot];
        }
    }
    fclose(infile);
    return true;
}

byte initGuild() {
    initGuildMem();
    return loadGuild();
}

void initGuildMem(void) {
    static unsigned int sizeBytes= 0;
    sizeBytes= GUILDSIZE * sizeof(character);
    printf("guild size is %x bytes\n",sizeBytes);
    guild= (character *)malloc(sizeBytes);
    if (guild == NULL) {
        puts("???fatal: no memory for guild");
        exit(0);
    }
    bzero(guild, sizeBytes);
}