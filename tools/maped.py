#!/usr/bin/env python3

import locale
import curses
import curses.textpad
import collections
import os
import pickle
import sys

from copy import deepcopy


class mapElement:
    pass


class mapEditor():

    kMapWinWidth = 20
    kMapWinHeight = 12
    mapWinWidth = 70
    mapWinHeight = 20
    kLowerTop = kMapWinHeight+4
    kScrollMargin = 2

    kDisplayCharacters = [
        # ---------- dungeon tiles ---------------
        ['.', 'D space/floor'],  # 0 : space/floor
        [u"\u25c6", 'D item'],  # 1 : item = diamond
        # 2 : vertical line (door)
        [u"\u007C", 'D vertical door'],
        # 3 : horizontal line (door)
        [u"\u2015", 'D horizontal door'],
        [u"\u2588", 'D wall'],  # 4 : wall = solid block
        # ------------ outdoor tiles -------------
        [',', 'O grass'],  # 5 : grass
        ['%', 'O sand'],  # 6 : sand
                          ['#', 'O stone path'],  # 7 : stone path
                          ['t', 'O small trees'],  # 8 : small trees
                          ['T', 'O large trees'],  # 9 : large trees
        # 10 : small water
                          ['w', 'O small water'],
                          ['W', 'O large water'],                # 11 : lg water
        # 12 : sm mountain
                          ['^', 'O hills'],
        # 13 : lg mountain
                          ['M', 'O mountains'],
                          ['c', 'O village'],                    # 14 : village
                          ['C', 'O castle'],                     # 15 : castle
                          ['i', 'O inn'],                        # 16 : inn
                          ['d', 'O dungeon'],                    # 17 : dungeon
                          ['b', 'O bridge']                      # 18 : bridge
    ]

    def setupEmptyMap(self):

        self.map = []
        self.feels = [""]
        self.routines = [[0, 0, 0, 0, 0, 0, 0, 0]]
        self.copyMapElement = None

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

        self.cursorX = 0
        self.cursorY = 0
        self.originX = 0
        self.originY = 0
        self.startX = 0
        self.startY = 0

        self.mapWidth = 32
        self.mapHeight = 32
        self.currentFilename = sys.argv[1]


        curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK)
        curses.init_pair(2, curses.COLOR_CYAN, curses.COLOR_BLACK)

        # init map structure
        self.setupEmptyMap()

    def resizeWindows(self):
        try:
            height,width = self.stdscr.getmaxyx()
            self.rawMapWinHeight = height-4
            self.rawMapWinWidth = width

            if self.rawMapWinHeight > self.mapHeight+2:
                self.rawMapWinHeight = self.mapHeight+2

            if self.rawMapWinWidth > self.mapWidth+2:
                self.rawMapWinWidth = self.mapWidth+2

            self.kMapWinWidth = self.rawMapWinWidth-2
            self.kMapWinHeight = self.rawMapWinHeight-2

            self.mapwin = self.stdscr.subwin(
               self.rawMapWinHeight, self.rawMapWinWidth, 1, 0)

            self.topwin = self.stdscr.subwin(
                1, width, 0, 0
            )

            self.bottomwin = self.stdscr.subwin(
                3, width, self.rawMapWinHeight+1, 0
            )
            
            self.mapwin.keypad(True)

            self.helpwin = self.stdscr.subwin(
                self.kMapWinHeight+2, 0, 3, self.kMapWinWidth+4)
        except Exception as e:
            curses.curs_set(0)
            self.stdscr.addstr("No room for UI. Aborting.\n"
                               "Please provide a screen with at least 80x24 characters.\n\n"
                               "- press any key -")
            self.stdscr.getch()
            exit(127)


    def refreshStatus(self):
        e = self.getCurrentMapEntry()
        self.bottomwin.erase()
        self.topwin.move(0, 0)
        self.topwin.clrtoeol()
        self.topwin.addstr(0, 0, "maped 1.0 <")
        self.topwin.addstr(self.currentFilename)
        self.topwin.addstr("> ")
        self.topwin.noutrefresh()

        x = self.originX + self.cursorX - 1
        y = self.originY + self.cursorY - 1
        self.bottomwin.move(0,0)
        self.bottomwin.addstr("x,y: "+str(x)+","+str(y))
        self.bottomwin.addstr(" startX,Y: "+str(self.startX)+","+str(self.startY))
        self.bottomwin.addstr(" imp: "+str(e.impassable))
        self.bottomwin.noutrefresh()

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
                if gX<self.mapWidth and gY<self.mapHeight:
                    v = self.map[gX][gY].initiallyVisible
                    d = self.map[gX][gY].mapElementID
                    if (v == True):
                        cp = 2
                    else:
                        cp = 1
                    self.mapwin.addstr(
                        y+1, x+1, self.kDisplayCharacters[d][0], curses.color_pair(cp))
                else:
                    d=0
                    cp=1
                

    def getCurrentMapEntry(self):
        x = self.originX + self.cursorX - 1
        y = self.originY + self.cursorY - 1
        e = self.map[x][y]
        return e

    def getUserInput(self, prompt):
        self.bottomwin.erase()
        self.bottomwin.addstr(0, 0, prompt)
        curses.echo()
        userInput = self.bottomwin.getstr(32)
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
        self.startX = self.cursorX+self.originX - 1
        self.startY = self.cursorY+self.originY - 1

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
        if self.copyMapElement is not None:
            self.getCurrentMapEntry().mapElementID = self.copyMapElement.mapElementID
            self.getCurrentMapEntry().initiallyVisible = self.copyMapElement.initiallyVisible
            self.getCurrentMapEntry().impassable = self.copyMapElement.impassable
            self.cursorX += 1

    def goto(self):
        self.bottomwin.erase()
        self.bottomwin.move(0, 0)
        inCoords = self.getUserInput("goto (x,y):")
        try:
            splitInCoords = inCoords.decode().split(',')    # python3, you SUCK!
            self.cursorX = int(splitInCoords[0])+1
            self.cursorY = int(splitInCoords[1])+1
            self.bottomwin.clear()
        except:
            pass

    def loadMap(self):
        try:
            loadFilename = self.currentFilename
            infile = open(loadFilename, "br")
            mdata = pickle.load(infile)
            self.mapWidth = mdata["width"]
            self.mapHeight = mdata["height"]
            self.startX = mdata["startX"]
            self.startY = mdata["startY"]
            self.map = mdata["map"]
            self.resizeWindows()
            infile.close()
        except Exception as e:
            self.bottomwin.clear()
            self.bottomwin.addstr(0, 0, f"Error loading map: {str(e)}")
            self.bottomwin.refresh()
            self.bottomwin.getch()

    def loadOrCreateMap(self):
        if (os.path.exists(self.currentFilename)):
            self.loadMap()
        else:
            self.newMap()

    def _saveMap(self, fname=""):
        if (fname):
            saveFilename = fname
        else:
            saveFilename = self.getUserInput("Save file:")
        saveFilename = self.currentFilename
        self.stdscr.addstr("\nSaving...")
        mdata = {
            "width": self.mapWidth,
            "height": self.mapHeight,
            "startX": self.startX,
            "startY": self.startY,
            "map": self.map,
        }
        outfile = open(saveFilename, "bw")
        pickle.dump(mdata, outfile)
        outfile.close()
        self.bottomwin.clear()
        self.bottomwin.addstr(0, 0,
                           "\nfile saved.\n- press any key -")
        self.bottomwin.refresh()
        self.bottomwin.getch()
        self.redrawStdEditorScreen()

    def saveMap(self):
        if (self.currentFilename):
            self._saveMap(self.currentFilename)
        else:
            self._saveMap("")

    def saveMapAs(self):
        self._saveMap("")

    def fillMap(self):
        elem = self.getCurrentMapEntry()
        for y in range(self.mapWidth):
            for x in range(self.mapHeight):
                self.map[y][x].initiallyVisible = elem.initiallyVisible
                self.map[y][x].mapElementID = elem.mapElementID
                self.map[y][x].impassable = elem.impassable
                self.map[y][x].startOpcodeIndex = elem.startOpcodeIndex

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

        newMap = []
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
            self.stdscr.addstr("\n\n** new map \"" +
                               self.currentFilename+"\" **\n")
            self.stdscr.addstr("Width (16-128): ")
            self.stdscr.refresh()
            curses.echo()
            try:
                width = int(self.stdscr.getstr(4))
                self.stdscr.addstr("Height (16-128): ")
                height = int(self.stdscr.getstr(4))
            except:
                curses.noecho()
                self.stdscr.addstr("-- aborted. press any key --")
                self.stdscr.getch()
                exit(0)
            curses.noecho()
        self.mapHeight = height
        self.mapWidth = width
        self.setupEmptyMap()
        self.resizeWindows()


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
                            "[p] start pos  [F] fill  [G] goto")
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
            't': self.trimMap,
            'F': self.fillMap,
            'G': self.goto
        }

        stopEd = 0
        self.cursorX = 1
        self.cursorY = 1

        self.loadOrCreateMap()
        self.redrawStdEditorScreen()

        while 0 == stopEd:
            self.refreshMap()
            self.refreshStatus()
            self.mapwin.move(self.cursorY, self.cursorX)
            elem = self.kDisplayCharacters[self.getCurrentMapEntry(
            ).mapElementID]
            elemChar = elem[0]
            elemDesc = elem[1]
            self.bottomwin.move(1, 0)
            self.bottomwin.clrtoeol()
            self.bottomwin.addstr(1, 0, '>'+elemChar+'< '+elemDesc)
            self.bottomwin.noutrefresh()
            curses.doupdate()

            c = self.mapwin.getch()
            if c== curses.KEY_RESIZE:
                self.resizeWindows()
                self.redrawStdEditorScreen()
            else:
                func = edcmds.get(c, 0)
                if (func == 0):
                    func = edcmds.get(chr(c), 0)
                if (func != 0):
                    func()
                    self.checkScrollMap()
                    self.checkBounds()
            # self.stdscr.addstr(18, 0, str(c))
                pass


def runMaped(aWin):
    myMaped = mapEditor(aWin)
    myMaped.runEditor()


if len(sys.argv) != 2:
    print("usage: "+sys.argv[0]+" <path-to-mapfile.drm>")
    sys.exit(127)

locale.setlocale(locale.LC_ALL, '')
curses.wrapper(runMaped)
