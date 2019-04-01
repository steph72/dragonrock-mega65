#!/usr/bin/python3

import locale
import curses
import curses.textpad
import collections
import os
import pickle

from copy import deepcopy

class mapElement:
    pass

class mapEditor():

    kMapWinWidth = 20
    kMapWinHeight = 12

    kLowerTop = kMapWinHeight+4

    kScrollMargin = 2

    kDisplayCharacters = ['.',       # space/floor
                          u"\u25c6",  # item = diamond
                          u"\u007C",  # vertical line (door)
                          u"\u2015",  # horizontal line (door)
                          u"\u2588",  # wall = solid block
                          ]

    def setupEmptyMap(self):

        self.map = []
        self.feels = [""]
        self.routines = []
        self.copyMapElement = 0
        self.currentFilename = ""

        for y in range(self.mapWidth):
            self.map.append([])
            for x in range(self.mapHeight):
                newMapElement = mapElement()
                newMapElement.mapElementID = 4
                newMapElement.feelID = 0
                newMapElement.opcodeList = []
                self.map[y].append(newMapElement)


    def __init__(self, outwin):

        self.stdscr = outwin
        self.mapwin = self.stdscr.subwin(
            self.kMapWinHeight+2, self.kMapWinWidth+2, 2, 1)

        self.helpwin = self.stdscr.subwin(
            15, 50, 3, self.kMapWinWidth+4)

        self.mapwin.keypad(True)

        self.cursorX = 0
        self.cursorY = 0
        self.originX = 0
        self.originY = 0
        self.startX = 0
        self.startY = 0

        self.mapWidth = 32
        self.mapHeight = 32

        # init map structure

        self.setupEmptyMap()

    def showHelp(self):
        self.helpwin.erase()
        self.helpwin.addstr("editor commands:\n\n"
                            "SPC  : plot current element\n"
                            "+/-  : increase/decrease current element\n"
                            " c   : copy current element\n"
                            " E   : edit feeling\n"
                            " N   : create & set new feel ID\n"
                            " l   : load map\n"
                            " p   : set start position\n"
                            "s/S  : save map / save map as\n"
                            " x   : export map for ingame use\n"
                            "\nUse cursor keys to navigate the map.\n")
        self.helpwin.refresh()

    def refreshStatus(self):
        e = self.getCurrentMapEntry()
        self.clearLower()
        self.stdscr.move(0, 0)
        self.stdscr.clrtoeol()
        self.stdscr.addstr(0, 0, "maped 1.0 <")
        self.stdscr.addstr(self.currentFilename)
        self.stdscr.addstr("> ")
        x = self.originX + self.cursorX
        y = self.originY + self.cursorY
        self.stdscr.addstr(" x,y: "+str(x)+","+str(y))
        self.stdscr.addstr(" fID: "+str(e.feelID))
        self.stdscr.addstr(" startX,Y: "+str(self.startX)+","+str(self.startY))
        # refresh current feel
        self.stdscr.move(self.kLowerTop, 0)
        self.stdscr.clrtoeol()
        self.stdscr.addstr(self.kLowerTop, 0, self.feels[e.feelID])

    def checkScrollMap(self):
        if self.cursorX > self.kMapWinWidth-self.kScrollMargin:
            if self.originX + self.kMapWinWidth < self.mapWidth:
                self.originX += 1
                self.cursorX -= 1
        if self.cursorY > self.kMapWinHeight-self.kScrollMargin:
            if self.originY + self.kMapWinHeight < self.mapHeight:
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

    def feelsBytes(self):
        arr = bytearray()
        arr.extend(map(ord, "FEELS"))
        arr.append(len(self.feels))
        for i in self.feels:
            bytes = bytearray()
            bytes.extend(map(ord, i.swapcase()))
            bytes.append(0)
            arr.extend(bytes)
        return arr

    def opcodeBytes(self):
        arr = bytearray()
        arr.extend(map(ord, "OPCS"))
        arr.append(0)
        return arr

    def mapBytes(self):
        mapbytes = bytearray()
        mapbytes.extend(map(ord, "DR0"))
        mapbytes.append(self.mapWidth)
        mapbytes.append(self.mapHeight)
        mapbytes.append(self.startX)
        mapbytes.append(self.startY)
        for y in range(self.mapHeight):
            for x in range(self.mapWidth):
                currentMapElem = self.map[x][y]
                mID = currentMapElem.mapElementID
                fID = currentMapElem.feelID
                outbyte1 = mID or (fID << 3)
                outbyte2 = 0
                mapbytes.append(outbyte1)
                mapbytes.append(outbyte2)
        return mapbytes

    def export(self):
        self.stdscr.addstr("\nExporting map to mapdata/")
        self.stdscr.addstr(self.currentFilename)
        self.stdscr.addstr(".drm")
        self.stdscr.refresh()
        outfile = open(b"mapdata/"+self.currentFilename+b".drm", "wb")
        outfile.write(self.mapBytes())
        outfile.write(self.feelsBytes())
        outfile.write(self.opcodeBytes())
        outfile.close()
        self.stdscr.addstr("\nDone. Press any key.")
        self.stdscr.refresh()
        self.stdscr.getch()

    def getUserInput(self, prompt):
        self.clearLower()
        self.stdscr.addstr(self.kLowerTop, 0, prompt)
        curses.echo()
        self.stdscr.refresh()
        userInput = self.stdscr.getstr(32)
        curses.noecho()
        return userInput

    ###################### editor commands ########################

    def cursorUp(self):
        self.cursorY -= 1

    def cursorDown(self):
        self.cursorY += 1

    def cursorLeft(self):
        self.cursorX -= 1

    def cursorRight(self):
        self.cursorX += 1

    def setStartPosition(self):
        self.startX = self.cursorX
        self.startY = self.cursorY

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
        self.copyMapElement = deepcopy(self.getCurrentMapEntry())

    def pasteElem(self):
        if self.copyMapElement != 0:
            self.getCurrentMapEntry().mapElementID = self.copyMapElement.mapElementID
            self.getCurrentMapEntry().feelID = self.copyMapElement.feelID
            self.getCurrentMapEntry().opcodeList = self.copyMapElement.opcodeList

    def exportMap(self):
        if (not self.currentFilename):
            self.clearLower()
            self.stdscr.addstr(self.kLowerTop, 0, "No current filename\n"
                               "Please s)ave the mapfile before exporting.\n"
                               "- press any key -")
            self.stdscr.getch()
            self.clearLower()
        else:
            self.export()

    def loadMap(self):
        loadFilename = self.getUserInput("Load file:")
        self.stdscr.addstr("\nLoading...")
        infile = open(b"mapsrc/"+loadFilename+b".ds", "br")
        self.currentFilename = loadFilename
        mdata = pickle.load(infile)
        self.mapWidth = mdata["width"]
        self.mapHeight = mdata["height"]
        self.startX = mdata["startX"]
        self.startY = mdata["startY"]
        self.map = mdata["map"]
        self.feels = mdata["feels"]
        self.routines = mdata["routines"]
        self.refreshMap()
        infile.close()
        self.stdscr.addstr("\ndone.\n- press any key -")
        self.stdscr.refresh()
        self.stdscr.getch()

    def _saveMap(self, fname=""):
        if (fname):
            saveFilename = fname
        else:
            saveFilename = self.getUserInput("Save file:")
        self.currentFilename = saveFilename
        self.stdscr.addstr("\nSaving...")
        mdata = {
            "width": self.mapWidth,
            "height": self.mapHeight,
            "startX": self.startX,
            "startY": self.startY,
            "map": self.map,
            "feels": self.feels,
            "routines": self.routines
        }
        outfile = open(b"mapsrc/"+saveFilename+b".ds", "bw")
        pickle.dump(mdata, outfile)
        outfile.close()
        self.stdscr.addstr("\ndone.\n- press any key -")
        self.stdscr.refresh()
        self.stdscr.getch()

    def saveMap(self):
        if (self.currentFilename):
            self._saveMap(self.currentFilename)
        else:
            self._saveMap("")

    def saveMapAs(self):
        self._saveMap("")

    def newMap(self):
        width = 0
        height = 0
        while (width < 16 or width > 128 or height < 16 or height > 128):
            self.stdscr.erase()
            self.stdscr.addstr("\n\n** new map **\n")
            self.stdscr.addstr("Width (16-128): ")
            self.stdscr.refresh()
            curses.echo()
            width = int(self.stdscr.getstr(4))
            self.stdscr.addstr("Height (16-128): ")
            height = int(self.stdscr.getstr(4))
            curses.noecho()
        self.mapHeight = height
        self.mapWidth = width
        self.setupEmptyMap()

    def userStartup(self):
        self.stdscr.erase()
        self.stdscr.addstr("### maped v0.1a ###\n"
                           "stephan kleinert, 7turtles software, 2019\n\n"
                           "n)ew map or l)oad existing map?")
        self.stdscr.refresh()
        choice = ""
        while choice != ord('l') and choice != ord('n'):
            choice = self.stdscr.getch()
        if choice == ord('l'):
            self.loadMap()
        else:
            self.newMap()

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
            'c': self.copyElem,
            'x': self.exportMap,
            's': self.saveMap,
            'S': self.saveMapAs,
            'l': self.loadMap,
            'p': self.setStartPosition
        }

        stopEd = 0
        self.cursorX = 1
        self.cursorY = 1

        self.userStartup()
        self.stdscr.erase()

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
