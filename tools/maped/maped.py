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
    kLowerTop = kMapWinHeight+6
    kScrollMargin = 2

    kOpcodes = ["NOP", "NSTAT", "DISP", "WKEY"]

    kDisplayCharacters = ['.',        # 0 : space/floor
                          u"\u25c6",  # 1 : item = diamond
                          u"\u007C",  # 2 : vertical line (door)
                          u"\u2015",  # 3 : horizontal line (door)
                          u"\u2588",  # 4 : wall = solid block
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

    def stringForRoutinesEntry(self, entry):
        opc = entry[0]
        mnemo = self.kOpcodes[opc]
        if (opc == 1 or opc == 2):
            mnemo += " "+str(entry[1])
            f = self.feels[entry[1]].replace("\n", "")
            if len(f) > 40:
                f = f[0:40]+"..."
            mnemo += " ["+f+"]"
        lnk = entry[7]
        if (lnk != 0):
            mnemo += "-> "+str(lnk)
        return mnemo

    def opcodeListForIndex(self, opcIdx):
        list = []
        currentOpcode = self.routines[opcIdx]
        while True:
            list.append((opcIdx,currentOpcode))
            if currentOpcode[7] == 0:
                break
            opcIdx = currentOpcode[7]
            currentOpcode = self.routines[opcIdx]
        return list

    def editOpcode(self, opcIdx):
        newOpc = []
        oldOpc = self.routines[opcIdx]
        opcList = self.opcodeListForIndex(opcIdx)
        self.opWin.erase()
        self.opWin.move(0, 0)
        self.opWin.addstr("editing opcode #"+str(opcIdx)+"\n\n")
        self.opWin.addstr("CURRENT opcode is: " +
                          str(oldOpc)+"\n\n")
        self.opWin.addstr("Please enter new opcode with comma seperated parameters\n"
                          "(ommitted parameter entries will be taken over from old opc)\n\n")

        for i in range(len(self.kOpcodes)):
            self.opWin.addstr(i+8, 71, str(i))
            self.opWin.addstr(i+8, 74, self.kOpcodes[i])

        self.opWin.move(8, 0)
        curses.echo()
        self.opWin.refresh()
        userInput = self.opWin.getstr(32)
        userInput = userInput.decode()
        curses.noecho()
        elems = userInput.split(',')
        for i in elems:
            self.opWin.addstr(i)
            currentNum = 0
            try:
                currentNum = int(i)
            except:
                pass
            newOpc.append(currentNum)
        while len(newOpc) < 8:
            newOpc.append(oldOpc[len(newOpc)])
        self.routines[opcIdx] = newOpc

    def allOpcodesList(self):
        opcList = []
        for i in range(len(self.routines)):
            opcList.append((i, self.routines[i]))
        return opcList

    def enterOpcodeEditorAt(self, startOpcode, rCount=0):
        opcsPerPage = 16
        quitOpcodeEdit = 0
        currentL = 0
        currentPage = 0
        chosenOpcode = 0
        self.stdscr.erase()
        self.stdscr.move(0, 0)
        self.stdscr.addstr(0, 0, "maped 1.0 - ** OPCODE EDIT MODE **")
        while quitOpcodeEdit == 0:
            if (startOpcode == 0):
                opcList = self.allOpcodesList()
            else:
                opcList = self.opcodeListForIndex(startOpcode)
            numRoutines = len(opcList)
            numPages = numRoutines//opcsPerPage
            if chosenOpcode < 0:
                if currentPage > 0:
                    chosenOpcode = opcsPerPage-1
                    currentPage -= 1
                else:
                    chosenOpcode = 0
            rIndex = (currentPage*opcsPerPage)+chosenOpcode
            if rIndex > len(opcList)-1:
                chosenOpcode -=1
            if chosenOpcode >= opcsPerPage:
                currentPage += 1
                chosenOpcode = 0
            self.opWin.erase()
            self.stdscr.addstr(20, 0, "page "+str(currentPage)+" opc"+str(chosenOpcode))
            self.stdscr.addstr(21, 0, "[+/-] paging | [up/down] choose | [left/right] desc/asc | [RETURN] edit \n"
                                      "[d] delete | [a] append new opc | [x] exit")
            for i in range(0, opcsPerPage):
                rIndex = (currentPage*opcsPerPage)+i
                if rIndex < len(opcList):
                    currentEntry = opcList[rIndex]
                    self.opWin.addstr(i, 2, str(currentEntry[0]))
                    self.opWin.addstr(
                        i, 5, self.stringForRoutinesEntry(currentEntry[1]))
                    if (i == chosenOpcode):
                        self.opWin.addstr(i, 0, "->")
            self.opWin.refresh()
            rIndex = (currentPage*opcsPerPage)+chosenOpcode
            c = self.stdscr.getch()
            if (c == ord('+')):
                currentPage += 1
            elif (c == ord('-')) and currentPage > 0:
                currentPage -= 1
            elif c == 259:
                chosenOpcode -= 1
            elif c == 258:
                chosenOpcode += 1
            elif (c == ord('a')):
                lastOpc = opcList[rIndex][1]
                self.routines.append([0, 0, 0, 0, 0, 0, 0, 0])
                lastOpc[7] = len(self.routines)-1
            elif (c == 10):
                self.editOpcode(opcList[rIndex][0])
            elif (c == ord('x')):
                self.redrawStdEditorScreen()
                return
            elif (c == ord('d')):
                self.deleteOpcode(opcList[rIndex][0])
            elif (c == 261):
                # step into opcode
                opcIdx = opcList[rIndex][0]
                if (opcIdx!=0):
                    self.enterOpcodeEditorAt(opcIdx)
            elif (c == 260):
                if (startOpcode != 0):
                    return

    def deleteOpcode(self, opcodeNo):
        safeDelete = True
        for i in self.routines:
            if (i[7] == opcodeNo):      # something linking to this opc?
                safeDelete = False
                break
        for yElems in self.map:
            for xElems in yElems:       # opcode used in map?
                if xElems.startOpcodeIndex == opcodeNo:
                    safeDelete = False
                    break
        if not safeDelete:
            self.opWin.move(9, 0)
            self.opWin.addstr("\ncan't delete. opcode is in use.\n-key-")
            self.opWin.getch()
            return
        del self.routines[opcodeNo]
        # fix indices
        for i in self.routines:
            if i[7] > opcodeNo:
                i[7] -= 1
        for yElems in self.map:
            for xElems in yElems:
                if xElems.startOpcodeIndex > opcodeNo:
                    xElems.startOpcodeIndex -= 1
                    break

    def opcodeEditMode(self):
        opcList = []
        self.enterOpcodeEditorAt(0)

    def deleteFeel(self, feelNo):
        for i in self.routines:
            if (i[0] == 1 and i[1] == feelNo):
                self.stdscr.addstr("\ncan't delete. message is in use.\n-key-")
                self.stdscr.getch()
                return
        rm = self.feels[feelNo]
        self.feels.remove(rm)
        # reorder pointers if needed
        for i in self.routines:
            if (i[0] == 1 or i[1] == 1):
                if i[1] > feelNo:
                    i[1] -= 1
        self.stdscr.addstr("\nmessage deleted.\n-key-")
        self.stdscr.getch()

    def feelEditMode(self):
        quitFeelEdit = 0
        edFeelIdx = 1
        self.stdscr.erase()
        self.clearLower()
        self.stdscr.move(0, 0)
        self.stdscr.clrtoeol()
        self.stdscr.addstr(0, 0, "maped 1.0 - ** MESSAGE EDIT MODE **")
        self.helpwin.erase()
        self.stdscr.move(3, 0)
        self.stdscr.addstr("== message editor ==\n\n"
                           "+/- : choose msg to edit\n"
                           " n  : alloc new message\n"
                           " d  : delete current message\n"
                           "RET : edit chosen message\n"
                           " x  : exit msg editor\n"
                           )

        while quitFeelEdit == 0:
            self.clearLower()
            self.stdscr.addstr(self.kLowerTop, 0, self.feels[edFeelIdx])
            self.stdscr.move(1, 0)
            self.stdscr.clrtoeol()
            self.stdscr.addstr(1, 0, "Editing feel ID "+str(edFeelIdx))
            self.stdscr.move(self.kLowerTop, 0)
            self.stdscr.refresh()

            c = self.stdscr.getch()
            if c == ord('+'):
                edFeelIdx += 1
            elif c == ord('-'):
                edFeelIdx -= 1
            elif c == ord('x'):
                self.redrawStdEditorScreen()
                return
            elif c == ord('d'):
                self.deleteFeel(edFeelIdx)
            elif c == ord('n'):
                self.feels.append("newFeel "+str(len(self.feels)+1))
                edFeelIdx = len(self.feels)-1

            if edFeelIdx < 1:
                edFeelIdx = 1
            if edFeelIdx > len(self.feels)-1:
                edFeelIdx = len(self.feels)-1

            if c == 10:
                self.editFeel(edFeelIdx)

    def feelForElement(self, elem):
        if (elem.startOpcodeIndex == 0):
            return ""  # opcode 0 is always nop
        else:
            currentOpcode = self.routines[elem.startOpcodeIndex]
            while currentOpcode[7] != 0:
                currentOpcode = self.routines[currentOpcode[7]]

        if currentOpcode[0] == 1:
            return self.feels[currentOpcode[1]]
        else:
            return ""

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
        self.stdscr.addstr(" startX,Y: "+str(self.startX)+","+str(self.startY))
        self.stdscr.addstr(" imp: "+str(e.impassable))
        # refresh current feel
        self.stdscr.move(self.kLowerTop, 0)
        self.stdscr.clrtoeol()
        self.stdscr.addstr(self.kLowerTop, 0, self.feelForElement(e))
        self.stdscr.move(1, 0)
        self.stdscr.addstr("opcode: #"+str(e.startOpcodeIndex)+" [")
        for i in self.routines[e.startOpcodeIndex]:
            self.stdscr.addstr(" "+str(i))
        self.stdscr.addstr(" ]")

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
                    y+1, x+1, self.kDisplayCharacters[d], curses.color_pair(cp))

    def getCurrentMapEntry(self):
        x = self.originX + self.cursorX - 1
        y = self.originY + self.cursorY - 1
        e = self.map[x][y]
        return e

    def clearLower(self):
        for i in range(5):
            self.stdscr.move(self.kLowerTop+i, 0)
            self.stdscr.clrtoeol()

    def feelsBytes(self):
        arr = bytearray()
        arr.extend(map(ord, "FEELS"))
        arr.append(len(self.feels))
        for i in self.feels:
            stripped = i.rstrip()
            commobytes = bytearray()
            unixbytes = bytearray()
            unixbytes.extend(map(ord, stripped.swapcase()))
            for p in unixbytes:  # lf -> cr
                if (p == 10):
                    commobytes.append(13)
                else:
                    commobytes.append(p)
            commobytes.append(0)
            arr.extend(commobytes)
        return arr

    def opcodeBytes(self):
        arr = bytearray()
        arr.extend(map(ord, "OPCS"))
        arr.append(len(self.routines))
        for i in self.routines:
            bytes = bytearray(i)
            arr.extend(bytes)
        return arr

    def mapBytes(self):
        mapbytes = bytearray()
        mapbytes.append(self.mapWidth)
        mapbytes.append(self.mapHeight)
        mapbytes.append(self.startX)
        mapbytes.append(self.startY)
        for y in range(self.mapHeight):
            for x in range(self.mapWidth):
                currentMapElem = self.map[x][y]
                mID = currentMapElem.mapElementID
                outbyte1 = currentMapElem.mapElementID
                if currentMapElem.initiallyVisible:
                    outbyte1 = outbyte1 | 128
                if currentMapElem.impassable:
                    outbyte1 = outbyte1 | 32
                outbyte2 = currentMapElem.startOpcodeIndex
                mapbytes.append(outbyte1)
                mapbytes.append(outbyte2)
        return mapbytes

    def export(self):
        self.stdscr.addstr("\nExporting map to mapdata/")
        self.stdscr.addstr(self.currentFilename)
        self.stdscr.addstr(".drm")
        self.stdscr.refresh()
        outfile = open(b"mapdata/"+self.currentFilename+b".drm", "wb")
        idbytes = bytearray()
        idbytes.extend(map(ord, "DR0"))
        segment1 = self.mapBytes()
        segment2 = self.feelsBytes()
        segment3 = self.opcodeBytes()
        dlength = len(segment1) + len(segment2) + len(segment3)
        idbytes.append(dlength % 256)
        idbytes.append(dlength//256)
        outfile.write(idbytes)
        outfile.write(segment1)
        outfile.write(segment2)
        outfile.write(segment3)
        outfile.close()
        self.clearLower()
        self.stdscr.addstr(self.kLowerTop, 0, "\ndone.\n- press any key -")
        self.stdscr.refresh()
        self.stdscr.getch()
        self.redrawStdEditorScreen()

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
        self.startX = self.cursorX+self.originX
        self.startY = self.cursorY+self.originY

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

    def addOpcodeToElement(self, opcodeIndex, elem):
        if elem.startOpcodeIndex == 0:
            elem.startOpcodeIndex = opcodeIndex
        else:
            currentOpcode = self.routines[elem.startOpcodeIndex]
            while currentOpcode[7] != 0:
                currentOpcode = self.routines[currentOpcode[7]]
            currentOpcode[7] = opcodeIndex

    def editFeel(self, feelIdx):
        currentFeel = self.feels[feelIdx]
        self.clearLower()
        self.stdscr.move(self.kLowerTop, 0)
        self.stdscr.addstr("Edit feel, end with CTRL+G):")
        curses.textpad.rectangle(
            self.stdscr, self.kLowerTop+1, 0, self.kLowerTop+5, 42)
        inWin = self.stdscr.subwin(3, 40, self.kLowerTop+2, 1)
        inTextbox = curses.textpad.Textbox(inWin)
        inTextbox.stripspaces = False
        bytes = bytearray()
        bytes.extend(map(ord, currentFeel))
        for i in bytes:
            inTextbox.do_command(chr(i))
        self.stdscr.refresh()
        inTextbox.edit()
        aFeel = inTextbox.gather()
        del inTextbox
        del inWin
        self.feels[feelIdx] = aFeel

    def newOpcode(self):
        e=self.getCurrentMapEntry()
        self.routines.append([0,0,0,0,0,0,0,0])
        self.addOpcodeToElement(len(self.routines)-1,e)

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
        newFeelID = len(self.feels)-1
        self.routines.append([1, newFeelID, 0, 0, 0, 0, 0, 0])
        self.addOpcodeToElement(len(self.routines)-1, e)

    def copyElem(self):
        self.copyMapElement = deepcopy(self.getCurrentMapEntry())

    def pasteElem(self):
        if self.copyMapElement != 0:
            self.getCurrentMapEntry().mapElementID = self.copyMapElement.mapElementID
            self.getCurrentMapEntry().startOpcodeIndex = self.copyMapElement.startOpcodeIndex
            self.getCurrentMapEntry().initiallyVisible = self.copyMapElement.initiallyVisible
            self.getCurrentMapEntry().impassable = self.copyMapElement.impassable
            self.cursorX += 1

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
                            "[l] load  [s] save  [S] saveAs  [x] export\n"
                            "misc:\n"
                            "[n] new opcode  [N] new NSTAT+msg  [e] script editor\n"
                            "[m] msg editor  [p] start pos")
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
            'N': self.newFeelForCurrentElement,
            'n': self.newOpcode,
            'c': self.copyElem,
            'x': self.exportMap,
            's': self.saveMap,
            'S': self.saveMapAs,
            'l': self.loadMap,
            'm': self.feelEditMode,
            'e': self.opcodeEditMode,
            'v': self.toggleInitiallyVisible,
            'g': self.toggleImpassable,
            'p': self.setStartPosition
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
