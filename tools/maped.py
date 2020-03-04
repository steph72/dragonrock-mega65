#!python3

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

    kDisplayCharacters = [
                        # ---------- dungeon tiles ---------------
                          ['.', 'D space/floor'],               #  0 : space/floor
                          [u"\u25c6", 'D item'],                #  1 : item = diamond
                          [u"\u007C", 'D vertical door'],       #  2 : vertical line (door)
                          [u"\u2015", 'D horizontal door'],     #  3 : horizontal line (door)
                          [u"\u2588", 'D wall'],                #  4 : wall = solid block
                        # ------------ outdoor tiles -------------
                          [',','O grass'],                      #  6 : grass
                          ['%','O sand'],                       #  7 : sand
                          ['#','O stone path'],                 #  7 : stone path
                          ['t','O small trees'],                #  7 : small trees
                          ['T','O large trees'],                #  8 : large trees
                          ['w','O small water'],                #  9 : small water
                          ['W','O large water'],                # 10 : lg water
                          ['^','O small mountain'],             # 11 : sm mountain
                          ['M','O large mountain'],             # 12 : lg mountain
                          ['c','O village'],                    # 13 : village
                          ['C','O castle'],                     # 14 : castle
                          ['i','O inn'],                        # 15 : inn
                          ['d','O dungeon']                     # 16 : dungeon
                          ]

    def setupEmptyMap(self):

        self.map = []
        self.feels = [""]
        self.routines = [[0, 0, 0, 0, 0, 0, 0, 0]]
        self.copyMapElement = 0
        self.currentFilename = ""

        for y in range(self.mapWidth):
            self.map.append([])
            for x in range(self.mapHeight):
                newMapElement = mapElement()
                newMapElement.mapElementID = 4
                newMapElement.initiallyVisible = False
                newMapElement.impassable = True
                newMapElement.startOpcodeIndex = 0
                self.map[y].append(newMapElement)

    def __init__(self, outwin):

        self.stdscr = outwin

        self.mapwin = self.stdscr.subwin(
            self.kMapWinHeight+2, self.kMapWinWidth+2, 2, 1)

        self.helpwin = self.stdscr.subwin(
            18, 50, 3, self.kMapWinWidth+4)

        self.opWin = self.stdscr.subwin(19, 80, 2, 0)

        self.mapwin.keypad(True)

        self.cursorX = 0
        self.cursorY = 0
        self.originX = 0
        self.originY = 0
        self.startX = 0
        self.startY = 0

        self.mapWidth = 32
        self.mapHeight = 32

        curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK)
        curses.init_pair(2, curses.COLOR_CYAN, curses.COLOR_BLACK)

        # init map structure

        self.setupEmptyMap()

    def refreshStatus(self):
        e = self.getCurrentMapEntry()
        self.clearLower()
        self.stdscr.move(0, 0)
        self.stdscr.clrtoeol()
        self.stdscr.addstr(0, 0, "maped 1.0 <")
        self.stdscr.addstr(self.currentFilename)
        self.stdscr.addstr("> ")
        x = self.originX + self.cursorX - 1
        y = self.originY + self.cursorY - 1
        self.stdscr.addstr(" x,y: "+str(x)+","+str(y))
        self.stdscr.addstr(" startX,Y: "+str(self.startX)+","+str(self.startY))
        self.stdscr.addstr(" imp: "+str(e.impassable))

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
                v = self.map[gX][gY].initiallyVisible
                d = self.map[gX][gY].mapElementID
                if (v == True):
                    cp = 2
                else:
                    cp = 1
                self.mapwin.addstr(
                    y+1, x+1, self.kDisplayCharacters[d][0], curses.color_pair(cp))

    def getCurrentMapEntry(self):
        x = self.originX + self.cursorX - 1
        y = self.originY + self.cursorY - 1
        e = self.map[x][y]
        return e

    def clearLower(self):
        for i in range(5):
            self.stdscr.move(self.kLowerTop+i, 0)
            self.stdscr.clrtoeol()


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

    def nextLine(self):
        self.cursorX = 0
        self.originX = 0
        self.cursorY += 1

    def setStartPosition(self):
        self.startX = self.cursorX+self.originX -1
        self.startY = self.cursorY+self.originY -1

    def toggleInitiallyVisible(self):
        e = self.getCurrentMapEntry()
        e.initiallyVisible = not e.initiallyVisible
        self.cursorX += 1

    def toggleImpassable(self):
        e = self.getCurrentMapEntry()
        e.impassable = not e.impassable

    def increaseCurrentElementID(self):
        e = self.getCurrentMapEntry()
        if e.mapElementID < len(self.kDisplayCharacters)-1:
            e.mapElementID += 1

    def decreaseCurrentElementID(self):
        e = self.getCurrentMapEntry()
        if e.mapElementID > 0:
            e.mapElementID -= 1

    def copyElem(self):
        self.copyMapElement = deepcopy(self.getCurrentMapEntry())

    def pasteElem(self):
        if self.copyMapElement != 0:
            self.getCurrentMapEntry().mapElementID = self.copyMapElement.mapElementID
            self.getCurrentMapEntry().initiallyVisible = self.copyMapElement.initiallyVisible
            self.getCurrentMapEntry().impassable = self.copyMapElement.impassable
            self.cursorX += 1

    def loadMap(self):
        loadFilename = self.getUserInput("Load file (.drm appended automatically): mapsrc/")
        self.stdscr.addstr("\nLoading...")
        infile = open(b"mapsrc/"+loadFilename+b".drm", "br")
        self.currentFilename = loadFilename
        mdata = pickle.load(infile)
        self.mapWidth = mdata["width"]
        self.mapHeight = mdata["height"]
        self.startX = mdata["startX"]
        self.startY = mdata["startY"]
        self.map = mdata["map"]
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
        }
        outfile = open(b"mapsrc/"+saveFilename+b".drm", "bw")
        pickle.dump(mdata, outfile)
        outfile.close()
        self.clearLower()
        self.stdscr.addstr(self.kLowerTop, 0, "\ndone.\n- press any key -")
        self.stdscr.refresh()
        self.stdscr.getch()
        self.redrawStdEditorScreen()

    def saveMap(self):
        if (self.currentFilename):
            self._saveMap(self.currentFilename)
        else:
            self._saveMap("")

    def saveMapAs(self):
        self._saveMap("")
    
    def trimMap(self):
        width = 0
        height = 0
        while (width < 16 or width > self.mapWidth or height < 16 or height > self.mapHeight):
            self.stdscr.erase()
            self.stdscr.addstr("\n\n** trim map **\n")
            self.stdscr.addstr("new width (16-"+str(self.mapWidth)+"): ")
            self.stdscr.refresh()
            curses.echo()
            width = int(self.stdscr.getstr(4))
            self.stdscr.addstr("new height (16-"+str(self.mapHeight)+"): ")
            height = int(self.stdscr.getstr(4))
            curses.noecho()
   
        newMap=[]
        for y in range(width):
            newRow = []
            for x in range(height):
                newRow.append(self.map[y][x])
            newMap.append(newRow)
        self.map = newMap
        self.mapHeight = height
        self.mapWidth = width
        self.redrawStdEditorScreen()

    
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
        if (width < self.kMapWinWidth):
            self.kMapWinWidth = width
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

    def redrawStdEditorScreen(self):
        self.stdscr.erase()
        self.mapwin.border()
        self.mapwin.addstr(0, 1, "Map")
        self.showHelp()

    def showHelp(self):
        self.helpwin.erase()
        self.helpwin.addstr("moves:\n"
                            "[c] copy  [SPC] paste  [+/-] inc/dec \n"
                            "toggles:\n"
                            "[g] impassable  [v] visible\n"
                            "io:\n"
                            "[l] load  [s] save  [S] saveAs\n"
                            "misc:\n"
                            "[p] start pos")
        self.helpwin.refresh()

    def runEditor(self):

        edcmds = {
            259: self.cursorUp,
            258: self.cursorDown,
            260: self.cursorLeft,
            261: self.cursorRight,
            10: self.nextLine,
            '+': self.increaseCurrentElementID,
            '-': self.decreaseCurrentElementID,
            ' ': self.pasteElem,
            'c': self.copyElem,
            's': self.saveMap,
            'S': self.saveMapAs,
            'l': self.loadMap,
            'v': self.toggleInitiallyVisible,
            'g': self.toggleImpassable,
            'p': self.setStartPosition,
            't': self.trimMap
        }

        stopEd = 0
        self.cursorX = 1
        self.cursorY = 1

        self.userStartup()
        self.redrawStdEditorScreen()

        while 0 == stopEd:
            self.refreshMap()
            self.refreshStatus()
            self.mapwin.move(self.cursorY, self.cursorX)
            elem = self.kDisplayCharacters[self.getCurrentMapEntry().mapElementID]
            elemChar = elem[0]
            elemDesc = elem[1]
            self.stdscr.move(self.kLowerTop, 0)
            self.stdscr.clrtoeol()
            self.stdscr.addstr(self.kLowerTop,1,'>'+elemChar+'< '+elemDesc)
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
