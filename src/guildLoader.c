

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

character *guild;
character *party[PARTYSIZE];

byte loadGuild(void) {
    static FILE *infile;
    static byte i;
    static byte slot;

    /*
          So here's an absolutely baffling mystery: When reading the character
       generator from RAM is enabled in the TED, reading the guild crashes in 1
       out of 14 tries with a 'device not present' error. At least, it does so
       on my development plus/4 connected to an sd2iec device. It seems like
       reading characters from RAM does something to the serial bus timing or
       whatever.

          Since I haven't got the slightest idea of what's going on there, and
       since I want to be able to play the game on my real plus/4, the custom
       charset is disabled while loading things.

          If (in stark contrast to everthing else I've ever created) this game
       should find an audience, maybe someone who has the means and the
       knowledge can find out what the effing crap is happening here...

      */

    disableCustomCharset();

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
    enableCustomCharset();

    return true;
}

byte initGuild() {
    initGuildMem();
    return loadGuild();
}

void initGuildMem(void) {
    static unsigned int sizeBytes= 0;
    sizeBytes= GUILDSIZE * sizeof(character);
    printf("guild size is %x bytes\n", sizeBytes);
    guild= (character *)malloc(sizeBytes);
    if (guild == NULL) {
        puts("???fatal: no memory for guild");
        exit(0);
    }
    bzero(guild, sizeBytes);
}