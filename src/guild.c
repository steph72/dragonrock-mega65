
#include <cbm.h>
#include <conio.h>
#include <c64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "character.h"
#include "congui.h"
#include "guild.h"

character *guild;

static FILE *outfile;

void newGuildMember(byte city);
void _listGuildMembers(void);
void listGuildMembers(void);

void flagError(char *e);

// clang-format off
#pragma code-name(push, "OVERLAY2");
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
            if (isInParty(i)) {
                cputc('*');
            } else {
                cputc(' ');
            }
            cprintf("%2d %.10s", i + 1, tempChar->name);
            gotoxy(x + 14, y);
            cprintf("%s-%d", gClassesS[tempChar->aClass], tempChar->city + 1);
        }
    }
}

void listGuildMembers(void) {
    cg_titlec(COLOR_LIGHTBLUE, COLOR_GREEN, 0,
              "Guild Members");
    _listGuildMembers();
    cputsxy(0, 23, "-- key --");
    cgetc();
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

void cleanupParty(void) {
    byte i;
    for (i= 0; i < PARTYSIZE - 1; ++i) {
        if (party[i] == NULL) {
            if (party[i + 1] != NULL) {
                party[i]= party[i + 1];
                party[i + 1]= NULL;
            }
        }
    }
}

void dropFromParty(void) {
    static byte pm;

    cclearxy(0, 22, 40);
    cputsxy(2, 22, "Remove whom (0=cancel)");
    cursor(1);
    fgets(drbuf, 3, stdin);
    pm= atoi(drbuf);
    if (pm == 0)
        return;
    --pm;
    if (pm >= PARTYSIZE) {
        flagError("You wish!");
        return;
    }
    free(party[pm]);
    party[pm]= NULL;
    cleanupParty();
}

byte isInParty(byte guildIdx) {
    static byte i;
    for (i= 0; i < PARTYSIZE; i++) {
        if (party[i] && party[i]->guildSlot == guildIdx) {
            return true;
        }
    }
    return false;
}

void addToParty(void) {
    static char inbuf[3];
    signed char slot;
    unsigned char gmIndex;

    character *newPartyCharacter;

    cclearxy(0, 22, 40);
    slot= nextFreePartySlot();
    if (slot == -1) {
        flagError("no room in party");
        return;
    }
    cg_clear();
    cg_titlec(COLOR_BROWN, COLOR_YELLOW, 0, "Add guild member");

    _listGuildMembers();
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

    newPartyCharacter= malloc(sizeof(character));
    memcpy(newPartyCharacter, (void *)&guild[gmIndex], sizeof(character));
    party[slot]= newPartyCharacter;
}

void purgeGuildMember(void) {
    static char cnum[5];
    static byte idx;
    cg_titlec(COLOR_ORANGE, 2, 0, "Purge guild member");
    textcolor(COLOR_RED);
    _listGuildMembers();
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
    outfile= fopen("gdata", "w");
    fwrite(guild, GUILDSIZE * sizeof(character), 1, outfile);
    fclose(outfile);
}

void saveParty(void) {
    static byte i, c;
    outfile= fopen("pdata", "w");
    c= partyMemberCount();
    fputc(c, outfile);
    for (i= 0; i < c; ++i) {
        fwrite(party[i], sizeof(character), 1, outfile);
    }
    fclose(outfile);
}

byte loadGuild(void) {
    static FILE *infile;

    infile= fopen("gdata", "r");
    if (!infile) {
        return false;
    }
    fread(guild, GUILDSIZE * sizeof(character), 1, infile);
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
        while(1);
    }
    bzero(guild, sizeBytes);
}

// clang-format off
#pragma code-name(pop);
// clang-format on
