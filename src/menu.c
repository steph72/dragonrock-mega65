#include "menu.h"
#include <stddef.h>
#include <stdio.h>

char **_menuEntries;
long lastMenuChecksum;

byte _menuEntriesCount;
signed char _menuSelectedEntry;
static byte _mI;

#define i _mI

byte refreshMenuH(byte x, byte y) {
    cg_gotoxy(x, y);
    for (i= 0; i < _menuEntriesCount; ++i) {
        cg_revers(_menuSelectedEntry == i);
        cg_puts(_menuEntries[i]);
        cg_revers(0);
        cg_puts(" ");
    }
}

byte refreshMenuV(byte x, byte y) {
    for (i= 0; i < _menuEntriesCount; ++i) {
        cg_gotoxy(x, y + i);
        cg_revers(_menuSelectedEntry == i);
        cg_puts(_menuEntries[i]);
        cg_revers(0);
        cg_puts(" ");
    }
}

#undef i

byte runMenu(char *entries[], byte x, byte y, byte vertical,
             byte enableNumberShortcuts) {
    byte quitMenu;
    char menuCmd;
    long checksum;
    char *menuEntry;

    _menuEntries= entries;
    _menuEntriesCount= 0;
    checksum= 0;

    quitMenu= false;
    while (menuEntry= _menuEntries[_menuEntriesCount++]) {
        checksum+= (long)menuEntry;
    };

    _menuEntriesCount--;

    if (checksum != lastMenuChecksum) {
        _menuSelectedEntry= 0;
        lastMenuChecksum= checksum;
    }

    if (!vertical) {
        cg_block_raw(x, y, gScreenColumns - 1, y, 32, 0);
    }

    while (!quitMenu) {
        if (vertical) {
            refreshMenuV(x, y);
        } else {
            refreshMenuH(x, y);
        }

        menuCmd= cg_getkey();

        if (menuCmd >= '1' && menuCmd <= '6' && enableNumberShortcuts) {
            return 100 + (menuCmd - '1');
        }

        switch (menuCmd) {
        case 157:
        case 145:
            _menuSelectedEntry--;
            break;

        case 17:
        case 29:
            _menuSelectedEntry++;
            break;

        case 13:
            quitMenu= true;

        default:
            break;
        }
        if (_menuSelectedEntry >= _menuEntriesCount) {
            _menuSelectedEntry= 0;
        } else if (_menuSelectedEntry < 0) {
            _menuSelectedEntry= _menuEntriesCount - 1;
        }
    }
    return _menuSelectedEntry;
}