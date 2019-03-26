#!/usr/bin/python3

import locale
import curses
import curses.textpad
import collections

from copy import deepcopy


class mapEditor():

    kMapWidth = 32
    kMapHeight = 32
    kMapWinWidth = 20
    kMapWinHeight = 12

    kLowerTop = kMapWinHeight+4

    kScrollMargin = 2

    kDisplayCharacters = ['.',       # space/floor
                          u"\u25c6",  # item = diamond
                          'd',       # door = d
                          u"\u2588"  # wall = solid block
                          ]

    def __init__(self, outwin):

        self.stdscr = outwin
        self.mapwin = self.stdscr.subwin(
            self.kMapWinHeight+2, self.kMapWinWidth+2, 2, 1)

        self.helpwin = self.stdscr.subwin(
            12, 50, 2, self.kMapWinWidth+4)

        self.mapwin.keypad(True)

        self.cursorX = 0
        self.cursorY = 0
        self.originX = 0
        self.originY = 0

        # init map structure

        self.map = []
        self.feels = [""]
        self.routines = []
        self.copytuple = 0

        for y in range(self.kMapWidth):
            self.map.append([])
            for x in range(self.kMapHeight):
                newtuple = collections.namedtuple(
                    'mapElement', 'mapElementID,feelID,opcodeList')
                newtuple.mapElementID = 3
                newtuple.feelID = 0
                newtuple.opcodeList = []
                self.map[y].append(newtuple)

    def showHelp(self):
        self.helpwin.addstr("editor commands:\n\n"
                            "SPC  : plot current element\n"
                            "+/-  : increase/decrease current element\n"
                            " E   : edit feeling\n"
                            " N   : create & set new feel ID\n"
                            " l   : load map\n"
                            " s   : save map\n"
                            " x   : export map for ingame use\n"
                            "\n"
                            "Use cursor keys to navigate the map.\n")

    def refreshStatus(self):
        e = self.getCurrentMapEntry()
        self.clearLower()
        self.stdscr.move(0, 0)
        self.stdscr.clrtoeol()
        self.stdscr.addstr(0, 0, "maped 1.0 ")
        x = self.originX + self.cursorX
        y = self.originY + self.cursorY
        self.stdscr.addstr(" x: "+str(x))
        self.stdscr.addstr(" y: "+str(y))
        self.stdscr.addstr(" fID: "+str(e.feelID))
        # refresh current feel
        self.stdscr.move(self.kLowerTop, 0)
        self.stdscr.clrtoeol()
        self.stdscr.addstr(self.kLowerTop, 0, self.feels[e.feelID])

    def checkScrollMap(self):
        if self.cursorX > self.kMapWinWidth-self.kScrollMargin:
            if self.originX + self.kMapWinWidth < self.kMapWidth:
                self.originX += 1
                self.cursorX -= 1
        if self.cursorY > self.kMapWinHeight-self.kScrollMargin:
            if self.originY + self.kMapWinHeight < self.kMapHeight:
                self.originY += 1
                self.cursorY -= 1
        if self.cursorX < self.kScrollMargin+1:
            if self.originX > 0:
                self.originX -= 1
                self.cursorX += 1
        if self.cursorY < self.kScrollMargin+1:
            if self.originY > 0:
                self.originY -= 1
                self.cursorY += 1

    def checkBounds(self):
        if self.cursorX < 1:
            self.cursorX = 1
        elif self.cursorX > self.kMapWinWidth:
            self.cursorX = self.kMapWinWidth
        if self.cursorY < 1:
            self.cursorY = 1
        elif self.cursorY > self.kMapWinHeight:
            self.cursorY = self.kMapWinHeight

    def refreshMap(self):
        for x in range(self.kMapWinWidth):
            for y in range(self.kMapWinHeight):
                gX = x + self.originX
                gY = y + self.originY
                d = self.map[gX][gY].mapElementID
                self.mapwin.addstr(
                    y+1, x+1, self.kDisplayCharacters[d])

    def getCurrentMapEntry(self):
        x = self.originX + self.cursorX - 1
        y = self.originY + self.cursorY - 1
        e = self.map[x][y]
        return e

    def clearLower(self):
        for i in range(7):
            self.stdscr.move(self.kLowerTop+i, 0)
            self.stdscr.clrtoeol()

    ###################### editor commands ########################

    def cursorUp(self):
        self.cursorY -= 1

    def cursorDown(self):
        self.cursorY += 1

    def cursorLeft(self):
        self.cursorX -= 1

    def cursorRight(self):
        self.cursorX += 1

    def increaseCurrentElementID(self):
        e = self.getCurrentMapEntry()
        if e.mapElementID < len(self.kDisplayCharacters)-1:
            e.mapElementID += 1

    def decreaseCurrentElementID(self):
        e = self.getCurrentMapEntry()
        if e.mapElementID > 0:
            e.mapElementID -= 1

    def newFeelForCurrentElement(self):
        e = self.getCurrentMapEntry()
        self.stdscr.move(self.kLowerTop, 0)
        self.stdscr.addstr("New feel (max. 3 lines, end with CTRL+G):")
        curses.textpad.rectangle(
            self.stdscr, self.kLowerTop+1, 0, self.kLowerTop+5, 42)
        inWin = self.stdscr.subwin(3, 40, self.kLowerTop+2, 1)
        inTextbox = curses.textpad.Textbox(inWin)
        self.stdscr.refresh()
        inTextbox.edit()
        aFeel = inTextbox.gather()
        del inTextbox
        del inWin
        self.feels.append(aFeel)
        e.feelID = len(self.feels)-1

    def copyElem(self):
        self.copytuple = deepcopy(self.getCurrentMapEntry())

    def pasteElem(self):
        if self.copytuple != 0:
            self.getCurrentMapEntry().mapElementID = self.copytuple.mapElementID
            self.getCurrentMapEntry().feelID = self.copytuple.feelID
            self.getCurrentMapEntry().opcodeList = self.copytuple.opcodeList

    def runEditor(self):

        edcmds = {
            259: self.cursorUp,
            258: self.cursorDown,
            260: self.cursorLeft,
            261: self.cursorRight,
            '+': self.increaseCurrentElementID,
            '-': self.decreaseCurrentElementID,
            ' ': self.pasteElem,
            'N': self.newFeelForCurrentElement,
            'c': self.copyElem
        }

        stopEd = 0
        self.cursorX = 1
        self.cursorY = 1
        self.mapwin.border()
        self.mapwin.addstr(0, 1, "Map")
        self.showHelp()

        while 0 == stopEd:
            self.refreshMap()
            self.refreshStatus()
            self.mapwin.move(self.cursorY, self.cursorX)
            self.stdscr.refresh()
            c = self.mapwin.getch()
            func = edcmds.get(c, 0)
            if (func == 0):
                func = edcmds.get(chr(c), 0)
            if (func != 0):
                func()
                self.checkScrollMap()
                self.checkBounds()
            self.stdscr.addstr(18, 0, str(c))
            pass


def runMaped(aWin):
    myMaped = mapEditor(aWin)
    myMaped.runEditor()


locale.setlocale(locale.LC_ALL, '')
curses.wrapper(runMaped)
