#include "encounter.h"
#include "globals.h"
#include "spell.h"
#include "utils.h"
#include <c64.h>
#include <unistd.h>

char *encounterActionNoun[]= {"Wait",  "Thrust", "Attack", "Slash",
                              "Parry", "Cast",   "Shoot"};

char *encounterActionVerb[]= {"waits",   "thrusts", "attacks", "slashes",
                              "parries", "casts",   "shoots"};

// clang-format off
#pragma code-name(push, "OVERLAY3");
// clang-format on

/*

           ==================================================
                          encounter overlay
           ==================================================

*/

encResult doEncounter() {
    bordercolor(COLOR_BLACK);
    bgcolor(COLOR_BLACK);
    textcolor(COLOR_RED);
    cputs("Encounter...");
    clearMonsters();
    cgetc();
    return encWon;
}

// clang-format off
#pragma code-name(pop);
// clang-format on
