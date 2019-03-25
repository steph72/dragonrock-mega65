#!/usr/bin/python3

import curses
import collections


class mapEditor():

    kMapWidth = 32
    kMapHeight = 32
    kMapWinWidth = 14
    kMapWinHeight = 14

    kScrollMargin = 2

    kDisplayCharacters = ['.', '*', 'd', '#']

    def __init__(self, outwin):

        self.stdscr = outwin
        self.mapwin = self.stdscr.subwin(
            self.kMapWinWidth+2, self.kMapWinHeight+2, 2, 1)

        self.mapwin.keypad(True)

        self.cursorX = 0
        self.cursorY = 0
        self.originX = 0
        self.originY = 0

        # init map structure

        self.map = []

        for y in range(self.kMapWidth):
            self.map.append([])
            for x in range(self.kMapHeight):
                newtuple = collections.namedtuple(
                    'mapElement', 'dispID,feelID,opcodeList')
                newtuple.dispID = 3
                newtuple.feelID = 0
                newtuple.opcodeList = []
                self.map[y].append(newtuple)

    def refreshStatus(self):
        self.stdscr.move(0, 0)
        self.stdscr.clrtoeol()
        self.stdscr.addstr(0, 0, "maped 1.0 ")
        x = self.originX + self.cursorX
        y = self.originY + self.cursorY
        self.stdscr.addstr(" cx: "+str(x))
        self.stdscr.addstr(" cy: "+str(y))

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
                d = self.map[gX][gY].dispID
                self.mapwin.addstr(
                    x+1, y+1, self.kDisplayCharacters[d])

    ###################### editor commands ########################

    def cursorUp(self):
        self.cursorY -= 1

    def cursorDown(self):
        self.cursorY += 1

    def cursorLeft(self):
        self.cursorX -= 1

    def cursorRight(self):
        self.cursorX += 1

    def runEditor(self):

        edcmds = {
            259: self.cursorUp,
            258: self.cursorDown,
            260: self.cursorLeft,
            261: self.cursorRight
        }

        stopEd = 0
        self.cursorX = 1
        self.cursorY = 1
        self.mapwin.border()
        self.mapwin.addstr(0, 1, "Map")

        while 0 == stopEd:
            self.refreshMap()
            self.refreshStatus()
            self.mapwin.move(self.cursorY, self.cursorX)
            self.stdscr.refresh()
            c = self.mapwin.getch()
            func = edcmds.get(c, 0)
            if (func != 0):
                func()
                self.checkScrollMap()
                self.checkBounds()
            self.stdscr.addstr(18, 0, str(c))
            pass

def runMaped(aWin):
    myMaped = mapEditor(aWin)
    myMaped.runEditor()


curses.wrapper(runMaped)
