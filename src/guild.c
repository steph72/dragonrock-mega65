
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "congui.h"
#include "guild.h"
#include "types.h"

character *guild;
character *party[PARTYSIZE];

void initGuildMem(void);
byte loadGuild(void);

void newGuildMember(byte city);
void _listGuildMembers(void);
void listGuildMembers(void);

void flagError(char *e);

// clang-format off
#pragma code-name(push, "OVERLAY1");
// clang-format on

void _listGuildMembers(void) {
    static byte i, x, y;
    static byte charsPerRow= GUILDSIZE / 2;
    character *tempChar;

    for (i= 0; i < GUILDSIZE; ++i) {
        if (guild[i].status != deleted) {
            tempChar= &guild[i];
            x= (20 * (i / charsPerRow));
            y= (4 + (i % charsPerRow));
            gotoxy(x, y);
            cprintf("%2d %.10s", i + 1, tempChar->name);
            gotoxy(x + 14, y);
            cprintf("%s-%d", gClassesS[tempChar->aClass], tempChar->city + 1);
        }
    }
}

void listGuildMembers(void) {
    cg_titlec(8, 5, 0, "Guild Members");
    _listGuildMembers();
    cputsxy(0, 23, "-- key --");
    cgetc();
}

void showCurrentParty(void) {
    static byte i, y;
    static character *c;
    gotoxy(0, 2);
    y= 2;
    for (i= 0; i < PARTYSIZE; i++) {
        if (party[i]) {
            c= party[i];
            ++y;
            gotoxy(0, y);
            cprintf("%d %s", i + 1, c->name);
            cputsxy(20, y, gRacesS[c->aRace]);
            cputsxy(24, y, gClassesS[c->aClass]);
            cputsxy(34, y, gStateDesc[c->status]);
        }
    }
}

void flagError(char *e) {
    textcolor(2);
    cursor(0);
    cclearxy(0, 22, 40);
    cputsxy(2, 22, e);
    textcolor(8);
    cputsxy(2, 23, "-- key --");
    cgetc();
}

void dropFromParty(void) {
    static char inbuf[3];
    static byte pm;

    cclearxy(0, 22, 40);
    cputsxy(2, 22, "Remove whom (0=cancel)");
    cursor(1);
    fgets(inbuf, 3, stdin);
    pm= atoi(inbuf);
    if (pm == 0)
        return;
    --pm;
    if (pm >= PARTYSIZE) {
        flagError("You wish!");
        return;
    }
    party[pm]= NULL;
}

byte isInParty(byte guildIdx) {
    static byte i;
    for (i= 0; i < PARTYSIZE; i++) {
        if (party[i] == &guild[guildIdx]) {
            return true;
        }
    }
    return false;
}

void addToParty(void) {
    static char inbuf[3];
    signed char slot;
    unsigned char gmIndex;
    cclearxy(0, 22, 40);
    slot= nextFreePartySlot();
    if (slot == -1) {
        flagError("no room in party");
        return;
    }
    cputsxy(2, 22, "Add which guild member (0=cancel)?");
    cursor(1);
    fgets(inbuf, 3, stdin);
    gmIndex= atoi(inbuf);
    if (gmIndex == 0) {
        return;
    }
    --gmIndex;
    if (gmIndex >= GUILDSIZE) {
        flagError("What is it with you?!");
        return;
    }
    if (guild[gmIndex].status == deleted) {
        flagError("nobody there");
        return;
    }
    if (isInParty(gmIndex)) {
        flagError("already in party");
        return;
    }

    party[slot]= &guild[gmIndex];
}

void purgeGuildMember(void) {
    static char cnum[5];
    static byte idx;
    static byte i;
    cg_titlec(8, 2, 0, "Purge guild member");
    textcolor(8);
    _listGuildMembers();
    textcolor(2);
    cputsxy(0, 22, "Purge which member (0=cancel)? ");
    fgets(cnum, 16, stdin);
    idx= atoi(cnum);
    if (idx == 0) {
        return;
    }
    idx--;
    if (idx >= GUILDSIZE) {
        flagError("Are you working in QA?");
        return;
    }
    if (isInParty(idx)) {
        flagError("Member is currently in the party!");
        return;
    }
    guild[idx].status= deleted;
}

signed char nextFreePartySlot(void) {
    signed char idx= -1;
    while (++idx < PARTYSIZE) {
        if (party[idx] == NULL) {
            return idx;
        }
    }
    return -1;
}

signed char nextFreeGuildSlot(void) {
    signed char idx= -1;
    while (++idx < GUILDSIZE) {
        if (guild[idx].status == deleted) {
            return idx;
        }
    }
    return -1;
}

void saveGuild(void) {
    static FILE *outfile;
    static byte i;
    clrscr();
    cg_borders();
    puts("\nPlease wait, "
         "saving the game...");
    outfile= fopen("guild", "w");
    fwrite(guild, GUILDSIZE * sizeof(character), 1, outfile);
    for (i= 0; i < PARTYSIZE; i++) {
        if (party[i]) {
            fputc(party[i]->guildSlot, outfile);
        } else {
            fputc(99, outfile);
        }
    }
    fclose(outfile);
    puts("\n\n...done.");
    cgetc();
}

byte loadGuild(void) {
    static FILE *infile;
    static byte i;
    static byte slot;
    infile= fopen("guild", "r");
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
    guild= (character *)malloc(sizeBytes);
    if (guild == NULL) {
        puts("???fatal: no memory for guild");
        exit(0);
    }
    bzero(guild, sizeBytes);
}

// clang-format off
#pragma code-name(pop);
// clang-format on
