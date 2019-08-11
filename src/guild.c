
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

extern character *guild;
extern character *party[];

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
            cprintf("%2d %.10s", i + 1, tempChar->name);
            gotoxy(x + 14, y);
            cprintf("%s-%d", gClassesS[tempChar->aClass], tempChar->city + 1);
        }
    }
}

void listGuildMembers(void) {
    cg_titlec(BCOLOR_CYAN | CATTR_LUMA3, BCOLOR_LEMON | CATTR_LUMA5, 0,
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
                // changed=true;
                party[i]= party[i + 1];
                party[i + 1]= NULL;
            }
        }
    }
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
    cleanupParty();
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
    clrscr();
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

    party[slot]= &guild[gmIndex];
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
    static FILE *outfile;
    static byte i;
    clrscr();
    cg_borders();
    puts("\nPlease wait, "
         "saving the game...");
    outfile= fopen("gdata", "w");
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

// clang-format off
#pragma code-name(pop);
// clang-format on
