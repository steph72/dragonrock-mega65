
#include <cbm.h>
//#include <conio.h>
#include <c64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "character.h"
#include "congui.h"
#include "guild.h"

character *guild;

FILE *outfile;

void newGuildMember(byte city);
void _listGuildMembers(void);
void listGuildMembers(void);

// clang-format off
#pragma code-name(push, "OVERLAY2");
#pragma rodata-name (push, "OVERLAY2")
#pragma local-strings (push,on)
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
            cg_gotoxy(x, y);
            if (isInParty(i)) {
                cg_putc('*');
            } else {
                cg_putc(' ');
            }
            cg_printf("%2d %.10s", i + 1, tempChar->name);
            cg_gotoxy(x + 14, y);
            cg_printf("%s-%d", gClassesS[tempChar->aClass], tempChar->city + 1);
        }
    }
}

void listGuildMembers(void) {
    cg_titlec(COLOR_GREEN, 0,
              "Guild Members");
    _listGuildMembers();
    cg_putsxy(0, 23, "-- key --");
    cg_getkey();
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

    cg_clearxy(0, 26, 40);
    cg_putsxy(2, 26, "Remove whom (0=cancel)");
    pm = cg_getnum(2);
    if (pm == 0)
        return;
    --pm;
    if (pm >= PARTYSIZE) {
        cg_displayErrorStatus("You wish!");
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
    static signed char slot;
    unsigned char gmIndex;

    character *newPartyCharacter;

    cg_clearxy(0, 26, 40);
    slot= nextFreePartySlot();
    if (slot == -1) {
        cg_displayErrorStatus("no room in party");
        return;
    }
    cg_clrscr();
    cg_titlec(COLOR_YELLOW, 0, "Add guild member");

    _listGuildMembers();
    cg_putsxy(0, 26, "Add which guild member (0=cancel)?");
    gmIndex = cg_getnum(2);
    if (gmIndex == 0) {
        return;
    }
    --gmIndex;
    if (gmIndex >= GUILDSIZE) {
        cg_displayErrorStatus("What is it with you?!");
        return;
    }
    if (guild[gmIndex].status == deleted) {
        cg_displayErrorStatus("nobody there");
        return;
    }
    if (isInParty(gmIndex)) {
        cg_displayErrorStatus("already in party");
        return;
    }

    newPartyCharacter= malloc(sizeof(character));
    memcpy(newPartyCharacter, (void *)&guild[gmIndex], sizeof(character));
    party[slot]= newPartyCharacter;
}

void purgeGuildMember(void) {
    static byte idx;
    cg_titlec(COLOR_RED, 0, "Purge guild member");
    cg_textcolor(COLOR_RED);
    _listGuildMembers();
    cg_putsxy(0, 26, "Purge which member (0=cancel)? ");
    idx = cg_getnum(2);
    if (idx == 0) {
        return;
    }
    idx--;
    if (idx >= GUILDSIZE) {
        cg_displayErrorStatus("Are you working in QA?");
        return;
    }
    if (isInParty(idx)) {
        cg_displayErrorStatus("Member is currently in the party!");
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
    FILE *infile;

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
        cg_fatal("no guild mem");
    }
    bzero(guild, sizeBytes);
}

// clang-format off
#pragma code-name(pop)
#pragma rodata-name(pop)
#pragma local-strings(pop)
// clang-format on
