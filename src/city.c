

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"
#include "config.h"
#include "congui.h"
#include "guild.h"
#include "types.h"

extern byte currentCity;
extern byte hasLoadedGame;
extern char outbuf[80];

// clang-format off
#pragma code-name(push, "OVERLAY1");
// clang-format on

void inspectCharacter(byte idx);

void inspectCharacter(byte idx) {
    character *ic;
    byte i;
    if (party[idx] == NULL) {
        return;
    }
    ic= party[idx];
    clrscr();
    revers(1);
    cputs(ic->name);
    revers(0);
    printf(" (%s, %s)\n", gRaces[ic->aRace], gClasses[ic->aClass]);
    for (i= 0; i < strlen(ic->name); ++i) {
        cputc('=');
    }
    puts("\n");
    for (i= 0; i < NUM_ATTRS; i++) {
        cputsxy(0, i + 3, gAttributesS[i]);
        cputsxy(3, i + 3, ":");
        gotoxy(5, i + 3);
        cprintf("%2d %s", ic->attributes[i],
                bonusStrForAttribute(ic->attributes[i]));
    }
    gotoxy(0, i + 4);
    printf(" HP: %d/%d\n", ic->aHP, ic->aMaxHP);
    printf(" MP: %d/%d\n", ic->aMP, ic->aMaxMP);
    gotoxy(16, 3);
    printf("   Age: %d", ic->age);
    gotoxy(16, 4);
    printf(" Level: %d", ic->level);
    gotoxy(16, 5);
    printf("    XP: %d", ic->xp);
    gotoxy(16, 6);
    printf(" Coins: %d", ic->gold);
    ic->weapon= 0;
    gotoxy(16, 8);
    printf("Weapon: %s", ic->inventory[ic->weapon]->name);
    gotoxy(16, 9);
    printf(" Armor: %s", ic->inventory[ic->armor]->name);
    gotoxy(16, 10);
    printf("Shield: %s", ic->inventory[ic->shield]->name);
    gotoxy(0, 13);
    printf("Inventory:\n");
    for (i= 0; i < INV_SIZE; i++) {
        gotoxy(20 * (i / (INV_SIZE / 2)), 15 + (i % (INV_SIZE / 2)));
        printf("%2d : %s", i, ic->inventory[i]->name);
    }
    cgetc();
}

void runGuildMenu(void) {
    const char menu[]= "  L)ist guild members  T)raining\n"
                       "  N)ew guild member    S)pells\n"
                       "  P)urge guild member\n"
                       "  A)dd to party\n"
                       "  D)rop from party\n"
                       "  eX)it guild\n";

    static unsigned char cmd;
    static unsigned char quitGuild;

    quitGuild= 0;
    sprintf(outbuf, "%s Guild", gCities[currentCity]);

    while (!quitGuild) {
        cg_titlec(8, 5, 1, outbuf);
        showCurrentParty();
        gotoxy(0, 14);
        puts(menu);
        cputsxy(2, 22, "Command:");
        cursor(1);
        do {
            cmd= cgetc();
        } while (strchr("lnpadxts123456", cmd) == NULL);

        cursor(0);

        if (cmd >= '1' && cmd <= '6') {
            inspectCharacter(cmd - '1');
        }

        switch (cmd) {

        case 'a':
            addToParty();
            break;

        case 'd':
            dropFromParty();
            break;

        case 'n':
            newGuildMember(currentCity);
            break;

        case 'l':
            listGuildMembers();
            break;

        case 'p':
            purgeGuildMember();
            break;

        case 'x':
            return;
            break;

        default:
            break;
        }
    }
}

void runCityMenu(void) {
    const char menu[]= " Go to\tA)rmory G)uild M)ystic\n"
                       "\tI)nn    B)ank  L)eave town\n\n"
                       "\tC)ast spell\n"
                       "\tU)se item\n"
                       "\tS)ave game\n";

    static unsigned char cmd;
    static unsigned char quitCity;

    quitCity= 0;
    while (!quitCity) {
        sprintf(outbuf, "%s (%d)", gCities[currentCity], currentCity + 1);
        cg_titlec(6, 5, 1, outbuf);
        showCurrentParty();
        gotoxy(0, 14);
        puts(menu);
        cputsxy(8, 21, "Command:");
        cursor(1);

        do {
            cmd= cgetc();
        } while (strchr("agmiblcus123456", cmd) == NULL);

        cursor(0);

        if (cmd >= '1' && cmd <= '6') {
            inspectCharacter(cmd - '1');
        }

        switch (cmd) {
        case 'l':
            quitCity= 1;
            break;

        case 'g':
            runGuildMenu();
            break;

        case 's':
            saveGuild();
            break;

        default:
            break;
        }
    }
}

void newGuildMember(byte city) {
    static byte i, c; // loop and input temp vars
    static byte race;
    static byte class;
    static attrT tempAttr[6];
    static attrT current;
    static signed char slot;
    static int tempHP;
    static int tempMP;
    static char cname[17];  // one more, because cc65 somehow adds
    static character *newC; // the "cr" sign to the finished string...

    static char top; // screen top margin

    const char margin= 14;
    const char delSpaces= 40 - margin;

    cg_titlec(8, 5, 0, "New Guild Member");

    slot= nextFreeGuildSlot();
    if (slot == -1) {
        textcolor(2);
        puts("\nSorry, the guild is full."
             "\nPlease purge some inactive members"
             "before creating new ones.\n\n--key--");
        cgetc();
        return;
    }

    top= 5;
    newC= &guild[slot];

    cputsxy(2, top, "      Race:");
    for (i= 0; i < NUM_RACES; i++) {
        gotoxy(margin, top + i);
        cprintf("%d - %s", i + 1, gRaces[i]);
    }
    cputsxy(margin, top + 1 + i, "Your choice: ");
    do {
        race= cgetc() - '1';
    } while (race >= NUM_RACES);
    for (i= top - 1; i < NUM_RACES + top + 3; cclearxy(margin, ++i, delSpaces))
        ;
    cputsxy(margin, top, gRaces[race]);

    ++top;

    cputsxy(2, top, "     Class:");
    for (i= 0; i < NUM_CLASSES; i++) {
        gotoxy(margin, top + i);
        cprintf("%d - %s", i + 1, gClasses[i]);
    }
    cputsxy(margin, top + 1 + i, "Your choice:");
    do {
        class= cgetc() - '1';
    } while (class >= NUM_CLASSES);
    for (i= top - 1; i < NUM_CLASSES + top + 3;
         cclearxy(margin, ++i, delSpaces))
        ;
    cputsxy(margin, top, gClasses[class]);

    top+= 2;

    cputsxy(2, top, "Attributes:");
    do {
        for (i= 0; i < 6; i++) {
            current= 7 + (rand() % 12) + gRaceModifiers[race][i];
            tempAttr[i]= current;
            cputsxy(margin, top + i, gAttributes[i]);
            gotoxy(margin + 13, top + i);
            cprintf("%2d %s", current, bonusStrForAttribute(current));
        }
        tempHP= 3 + (rand() % 8) + bonusValueForAttribute(tempAttr[0]);
        tempMP= 3 + (rand() % 8) + bonusValueForAttribute(tempAttr[1]);

        gotoxy(margin, top + i + 1);
        cprintf("Hit points   %2d", tempHP);

        gotoxy(margin, top + i + 2);
        cprintf("Magic points %2d", tempMP);

        cputsxy(margin, top + i + 4, "k)eep, r)eroll or q)uit? ");
        do {
            c= cgetc();
        } while (!strchr("rkq", c));
    } while (c == 'r');

    if (c == 'q')
        return;

    top= top + i + 4;
    cclearxy(0, top, 40);
    cputsxy(18, top + 1, "---------------");
    cputsxy(2, top, "Character name: ");
    fgets(cname, 17, stdin); // see above
    cname[strlen(cname) - 1]= 0;

    // copy temp char to guild
    newC->city= city;
    newC->guildSlot= slot;
    newC->status= alive;
    newC->aRace= race;
    newC->aClass= class;
    for (i= 0; i < NUM_ATTRS; i++) {
        newC->attributes[i]= tempAttr[i];
    }
    for (i= 0; i < INV_SIZE; i++) {
        newC->inventory[i]= &gItems[0]; // "empty" item
    }
    newC->aMaxHP= tempHP;
    newC->aHP= tempHP;
    newC->aMaxMP= tempMP;
    newC->aMP= tempMP;
    newC->level= 1;
    strcpy(newC->name, cname);
}

// clang-format off
#pragma code-name(pop);
// clang-format on