#!/usr/bin/python3

import sys
import pickle
import pyparsing as pp


class mapElement:
    pass

# --------------------------------------------------------------------------------------------


class mapCompiler:

    def __init__(self):

        # a list of labels in the form of "'labelName:' <line in source code>"
        self.gLabels = {} 

        # a mapping of source code line numbers to actual opcode indices
        self.gLinePosMapping = []
  

        self.gStringMapping = {}
        self.gCoordsMapping = {}
        self.gStrings = []
        self.gOpcodes = []

        self.mapWidth = 0
        self.mapHeight = 0
        self.startX = 0
        self.startY = 0
        self.map = []

    # low level map exporting

    def opcodeBytes(self):
        arr = bytearray()
        arr.extend(map(ord, "OPCS"))
        arr.append(len(self.gOpcodes))
        for i in self.gOpcodes:
            bytes = bytearray(i)
            arr.extend(bytes)
        return arr

    def feelsBytes(self):
        arr = bytearray()
        arr.extend(map(ord, "FEELS"))
        arr.append(len(self.gStrings))
        for i in self.gStrings:
            commobytes = bytearray()
            unixbytes = bytearray()
            i = i.replace("&&nl&&", "\n")
            unixbytes.extend(map(ord, i.swapcase()))
            for p in unixbytes:  # lf -> cr
                if (p == 10):
                    commobytes.append(10)
                    commobytes.append(13)
                else:
                    commobytes.append(p)
            commobytes.append(0)
            arr.extend(commobytes)
        return arr

    def mapElementByte(self, currentMapElem):
        mID = currentMapElem.mapElementID
        outbyte1 = currentMapElem.mapElementID
        if currentMapElem.initiallyVisible:
            outbyte1 = outbyte1 | 128
        if currentMapElem.impassable:
            outbyte1 = outbyte1 | 32
        return outbyte1

    def mapBytes(self):
        mapbytes = bytearray()
        mapbytes.append(self.mapWidth)
        mapbytes.append(self.mapHeight)
        mapbytes.append(self.startX)
        mapbytes.append(self.startY)
        for y in range(self.mapHeight):
            for x in range(self.mapWidth):
                currentMapElem = self.map[x][y]
                outbyte1 = self.mapElementByte(currentMapElem)
                outbyte2 = currentMapElem.startOpcodeIndex
                mapbytes.append(outbyte1)
                mapbytes.append(outbyte2)
        return mapbytes

    def export(self, filename):
        outfile = open(filename, "wb")
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

#######################################################################################

    def trim(self, lines):
        out = []
        lineNum = 0
        isReadingMultilineString = False
        currentMultilineString = ""
        for i in lines:
            lineNum += 1
            multilineQuotePos = i.find("\"\"\"")
            if (multilineQuotePos >= 0):
                if isReadingMultilineString:
                    # end reading multiline string
                    i = i.replace("\n", "")  # remove last newline
                    currentMultilineString += i
                    currentMultilineString = currentMultilineString.replace(
                        "\\n", "&&nl&&")
                    currentMultilineString = currentMultilineString.replace(
                        "\"\"\"", "\"")
                    currentMultilineString = currentMultilineString.replace(
                        "\n", "&&nl&&")
                    out.append((lineNum, currentMultilineString))
                    isReadingMultilineString = False
                    # print ("====>"+currentMultilineString+"<====")
                    continue
                else:
                    # start reading multiline string
                    currentMultilineString = ""
                    i = i.replace("\n", "")  # remove first newline
                    isReadingMultilineString = True

            if isReadingMultilineString:
                currentMultilineString += i
            else:
                commentPos = i.find(";")
                if (commentPos >= 0):
                    i = i[:commentPos]
                i = i.strip()
                if (len(i) > 0):
                    out.append((lineNum, i))

        return out

    ## trims labels from srcLines and builds label -> lineNumber mapping
    def scanAndTrimLabels(self, srcLines):
        returnLines = []
        print("Scanning labels")
        currentLabels = []  
        # go through source lines...
        for lineTupel in srcLines:
            lineNum = lineTupel[0]
            line = lineTupel[1]
            # is it a label?
            if line.endswith(":"):
                if (self.gLabels.get(line)):
                    print("error: duplicate label definition at line", lineNum)
                    print("       (original definition was at line " +
                          str(self.gLabels.get(line))+")")
                    exit(-1)
                # add to current labels
                currentLabels.append(line)
            else:
                # not a label? append to return lines...
                returnLines.append((lineNum, line))
                # ...and let last unresolved label point to this line
                for i in currentLabels:
                    self.gLabels[i] = lineNum
                # delete unresolved label list
                currentLabels = []
        return returnLines

    def loadMap(self, mapName):
        print("Reading map layout file", mapName)
        infile = open(mapName, "br")
        mdata = pickle.load(infile)
        self.mapWidth = mdata["width"]
        self.mapHeight = mdata["height"]
        self.startX = mdata["startX"]
        self.startY = mdata["startY"]
        self.map = mdata["map"]
        infile.close()

    def writeFileMessage(self, name, message):
        arr = bytearray()
        outstring = message.replace("&&nl&&", "\n")
        outstring = outstring.swapcase()
        commobytes = bytearray()
        unixbytes = bytearray()
        unixbytes.extend(map(ord, outstring))
        for p in unixbytes:  # lf -> cr
            if (p == 10):
                commobytes.append(13)
            else:
                commobytes.append(p)
        commobytes.append(0)
        arr.extend(commobytes)
        print("writing file message", name)
        outfile = open(name, "wb")
        outfile.write(commobytes)
        outfile.close()

    def buildStringsAndCoords(self, p_table):
        for i in p_table:
            line = i[0]
            src = i[1]
            if (src.metaCmd == "$"):
                self.gStringMapping[src.tMsgLabel] = len(self.gStrings)
                self.gStrings.append(src.tMessage)
            if (src.metaCmd == "&"):
                self.writeFileMessage(src.tMsgFile, src.tFileMessage)
            if (src.metaCmd == "defc"):
                x1 = int(src.tXValue)
                y1 = int(src.tYValue)
                x2 = x1
                y2 = y1
                if (src.tY2Value):
                    x2 = int(src.tX2Value)
                    y2 = int(src.tY2Value)
                self.gCoordsMapping[src.tCoordsLabel] = (x1, y1, x2, y2)
            if (src.metaCmd == "includemap"):
                self.loadMap(src.tMapName)

    def buildOpcodes(self, p_table):

        self.gLinePosMapping = {}
        opcodes = [(-1, [0, 0, 0, 0, 0, 0, 0, 0])]

        def checkString(aLabel, pline):
            if self.gStringMapping.get(aLabel) is None:
                print("error: can't find string \""+aLabel +
                      "\" at line "+str(pline.lineNum))
                exit(-1)

        # -------------- opcode factory --------------

        def opCreate_NOP(pline):
            return [0, 0, 0, 0, 0, 0, 0, 0]

        def opCreate_GOTO(pline):
            opc = [0, 0, 0, 0, 0, 0, 0, 0]
            opc[7] = "__DRLABEL__"+pline.tOpcLabel
            return opc

        def opCreate_NSTAT(pline):
            checkString(pline.tMsgLabel, pline)
            return [1, self.gStringMapping[pline.tMsgLabel], 0, 0, 0, 0, 0, 0]

        def opCreate_DISP(pline):
            checkString(pline.tMsgLabel, pline)
            opc = [2, self.gStringMapping[pline.tMsgLabel], 0, 0, 0, 0, 0, 0]
            if (pline.tClrFlag == True):
                opc[2] = 1
            return opc

        def opCreate_WKEY(pline):
            checkString(pline.tMsgLabel, pline)
            opc = [3, self.gStringMapping[pline.tMsgLabel], 0, 0, 0, 0, 0, 0]
            if (pline.tClrFlag == True):
                opc[2] = 1
            if (pline.tRegIndex):
                opc[3] = int(pline.tRegIndex)
            return opc

        def opCreate_YESNO(pline):
            opc = [4, 0, 0, 0, 0, 0, 0, 0]
            if (pline.tTrueOpcLabel):
                opc[1] = "__DRLABEL__"+pline.tTrueOpcLabel
            if (pline.tFalseOpcLabel):
                opc[2] = "__DRLABEL__"+pline.tFalseOpcLabel
            return opc

        def opCreate_YESNO_B(pline):
            opc = opCreate_YESNO(pline)
            opc[0] = 0x44
            return opc

        def opCreate_IFREG(pline):
            opc = [5, int(pline.tRegIndex), int(
                pline.tRegValue), 0, 0, 0, 0, 0]
            if (pline.tTrueOpcLabel):
                opc[3] = "__DRLABEL__"+pline.tTrueOpcLabel
            if (pline.tFalseOpcLabel):
                opc[4] = "__DRLABEL__"+pline.tFalseOpcLabel
            return opc

        def opCreate_IFREG_B(pline):
            opc = opCreate_IFREG(pline)
            opc[0] = 0x45
            return opc

        def opCreate_IFPOS(pline):
            opc = [6, int(pline.tItemID), 0, 0, int(pline.tRegIndex), 0, 0, 0]
            if (pline.tTrueOpcLabel):
                opc[2] = "__DRLABEL__"+pline.tTrueOpcLabel
            if (pline.tFalseOpcLabel):
                opc[3] = "__DRLABEL__"+pline.tFalseOpcLabel
            return opc

        def opCreate_IADD(pline):
            opc = [7, int(pline.tItemID), int(pline.tCharID), 0, 0, 0, 0, 0]
            if (pline.tTrueOpcLabel):
                opc[3] = "__DRLABEL__"+pline.tTrueOpcLabel
            if (pline.tFalseOpcLabel):
                opc[4] = "__DRLABEL__"+pline.tFalseOpcLabel
            return opc

        def opCreate_IADD_V(pline):
            opc = opCreate_IADD(pline)
            opc[0] = 0x87
            return opc

        def opCreate_ALTER(pline):
            opc = [8, 0, 0, 0, 0, 0, 0, 0]
            coords = self.gCoordsMapping.get(pline.tCoordsLabel)
            if not coords:
                print("error: cannot find coordinate mapping for",
                      pline.tCoordsLabel)
                exit(-1)
            opc[1] = coords[0]
            opc[2] = coords[1]
            opc[3] = "__DRLABEL__"+pline.tOpcLabel
            opc[4] = int(pline.tDungeonItemID)
            return opc

        def opCreate_REDRAW(pline):
            return [9, 0, 0, 0, 0, 0, 0, 0]

        def opCreate_ADDC(pline):
            opc = [0xa, 0, 0, 0, 0, 0, 0, 0]
            opc[1] = int(pline.tCoinsValue) % 255
            opc[2] = int(pline.tCoinsValue)//255
            return opc

        def opCreate_ADDC_V(pline):
            opc = opCreate_ADDC(pline)
            opc[0] = 0x8a
            return opc

        def opCreate_ADDE(pline):
            opc = [0x0b, 0, 0, 0, 0, 0, 0, 0]
            opc[1] = int(pline.tExpValue) % 255
            opc[2] = int(pline.tExpValue)//255
            return opc

        def opCreate_ADDE_V(pline):
            opc = opCreate_ADDE(pline)
            opc[0] = 0x8b
            return opc

        def opCreate_SETREG(pline):
            opc = [0x0c, int(pline.tRegIndex), int(
                pline.tRegValue), 0, 0, 0, 0, 0]
            return opc

        def opCreate_CLRENC(pline):
            opc = [0x0d, 0, 0, 0, 0, 0, 0, 0]
            return opc

        def opCreate_ADDENC(pline):
            opc = [0x0e, 0, 0, 0, 0, 0, 0, 0]
            opc[1] = int(pline.tMonsterID)
            opc[2] = int(pline.tMonsterLevel)
            opc[3] = int(pline.tMonsterCount)
            opc[4] = int(pline.tMonsterRow)
            return opc

        def opCreate_ADDENC1(pline):
            opc = opCreate_ADDENC0(pline)
            opc[0] = 0x8e
            return opc

        def opCreate_GOENC(pline):
            opc = [0x0f, 0, 0, 0, 0, 0, 0, 0]
            if (pline.tWinOpcLabel):
                opc[1] = "__DRLABEL__"+pline.tWinOpcLabel
            if (pline.tLoseOpcLabel):
                opc[2] = "__DRLABEL__"+pline.tLoseOpcLabel
            return opc

        def opCreate_ENTER_W(pline):
            opc = [0x10, 0, 0, 0, 0, 0, 0, 0]
            opc[1] = int(pline.tMapID)
            opc[2] = int(pline.tXValue)
            opc[3] = int(pline.tYValue)
            return opc

        def opCreate_ENTER_D(pline):
            opc = [0x30, 0, 0, 0, 0, 0, 0, 0]
            opc[1] = int(pline.tMapID)
            opc[2] = int(pline.tXValue)
            opc[3] = int(pline.tYValue)
            return opc

        lastOpcodeIndex = 0
        lastOpcode = []

        for i in p_table:
            lineNum = i[0]
            src = i[1]
            if src.metaCmd == "---":
                lastOpcode = []
            if not src.opcode:
                continue

            src.lineNum = lineNum
            opCreateFunc = "opCreate_"+src.opcode     # construct building function name
            newOpcode = locals()[opCreateFunc](src)   # ...and call it.
            opcodes.append((lineNum, newOpcode))

            newOpcodeIndex = len(opcodes)-1
            # add position to mapping
            self.gLinePosMapping[lineNum] = newOpcodeIndex

            if (lastOpcode):
                if (lastOpcode[7] == 0):
                    # link last opcode to current...
                    lastOpcode[7] = newOpcodeIndex

            # ...and remember this opcode for next link
            lastOpcode = newOpcode

        print("Calculating branch positions")
        
        for i in opcodes:
            labelJumpTable = []
            line = i[0]
            opcode = i[1]
            paramIdx = 0
            for k in opcode:
                if type(k) is str:
                    label = k[len("__DRLABEL__"):]+":"
                    labelLineNumber = self.gLabels.get(label)
                    try:
                        opcodeNumber = self.gLinePosMapping[labelLineNumber]
                    except:
                        print("cannot resolve ", label)
                        exit(0)
                    # print(label, labelLineNumber, opcodeNumber)
                    opcode[paramIdx] = opcodeNumber
                paramIdx += 1
        retList = []
        for i in opcodes:
            retList.append(i[1])
        return retList

    def parseScript(self, codeLines):

        print("Parsing script")

        p_numeric_value = pp.pyparsing_common.number()
        p_quoted_string = pp.QuotedString('"')
        p_TRUE = pp.CaselessKeyword("true").setParseAction(lambda tokens: True)
        p_FALSE = pp.CaselessKeyword(
            "false").setParseAction(lambda tokens: False)
        p_idchars = pp.Word(pp.alphanums+"_")

        p_boolean_literal = p_TRUE | p_FALSE

        value = p_numeric_value | p_quoted_string | p_boolean_literal | pp.Word(
            pp.alphanums)

        p_coordsLabel = p_idchars('tCoordsLabel')
        p_msgLabel = p_idchars(pp.alphanums)('tMsgLabel')
        p_opcLabel = pp.Word(pp.alphanums)('tOpcLabel')
        p_mapName = pp.Word(pp.alphanums)('tMapName')
        p_trueOpcLabel = pp.Word(pp.alphanums)('tTrueOpcLabel')
        p_falseOpcLabel = pp.Word(pp.alphanums)('tFalseOpcLabel')
        p_winOpcLabel = pp.Word(pp.alphanums)('tWinOpcLabel')
        p_loseOpcLabel = pp.Word(pp.alphanums)('tLoseOpcLabel')
        p_regIdx = pp.Word(pp.nums)('tRegIndex')
        p_regValue = pp.Word(pp.nums)('tRegValue')
        p_itemID = pp.Word(pp.nums)('tItemID')
        p_monsterID = pp.Word(pp.nums)('tMonsterID')
        p_monsterLevel = pp.Word(pp.nums)('tMonsterLevel')
        p_dungeonItemID = pp.Word(pp.nums)('tDungeonItemID')
        p_charID = pp.Word(pp.nums)('tCharID')
        p_mapID = pp.Word(pp.nums)('tMapID')
        p_xValue = pp.Word(pp.nums)('tXValue')
        p_yValue = pp.Word(pp.nums)('tYValue')
        p_x2Value = pp.Word(pp.nums)('tX2Value')
        p_y2Value = pp.Word(pp.nums)('tY2Value')
        p_coinsValue = pp.Word(pp.nums)('tCoinsValue')
        p_expValue = pp.Word(pp.nums)('tExpValue')

        p_keywords = (

            # opcodes

            pp.Keyword("NOP")('opcode')

            ^ pp.Keyword("GOTO")('opcode')+p_opcLabel

            ^ pp.Keyword("NSTAT")('opcode')+p_msgLabel

            ^ pp.Keyword("DISP")('opcode')+p_msgLabel+pp.Optional(","+p_boolean_literal('tClrFlag'))

            ^ (pp.Keyword("WKEY")('opcode')+p_msgLabel +
               pp.Optional(","+p_boolean_literal('tClrFlag')) +
               pp.Optional(","+p_regIdx))

            ^ (pp.Keyword("IFREG")('opcode')+p_regIdx+","+p_regValue+","+p_trueOpcLabel +
               pp.Optional(","+p_falseOpcLabel))

            ^ pp.Keyword("IFREG_B")('opcode')+p_regIdx+","+p_regValue+","+p_trueOpcLabel

            ^ pp.Keyword("SETREG")('opcode')+p_regIdx+","+p_regValue

            ^ pp.Keyword("YESNO")('opcode')+p_trueOpcLabel+pp.Optional(","+p_falseOpcLabel)

            ^ pp.Keyword("YESNO_B")('opcode')+p_trueOpcLabel+pp.Optional(","+p_falseOpcLabel)

            ^ (pp.Keyword("IFPOS")('opcode') + p_itemID +
               ","+p_trueOpcLabel+","+p_falseOpcLabel +
               ","+p_regIdx)

            ^ pp.Keyword("ADDC")('opcode') + p_coinsValue

            ^ pp.Keyword("ADDC_V")('opcode') + p_coinsValue

            ^ pp.Keyword("ADDE")('opcode') + p_expValue

            ^ pp.Keyword("ADDE_V")('opcode') + p_expValue

            ^ (pp.Keyword("IADD")('opcode')
               + p_itemID + ","
               + p_charID + ","
               + pp.Optional(","
                             + p_trueOpcLabel + ","
                             + p_falseOpcLabel
                             )
               )

            ^ (pp.Keyword("IADD_V")('opcode')
               + p_itemID + ","
               + p_charID
               + pp.Optional(","
                             + p_trueOpcLabel + ","
                             + p_falseOpcLabel
                             )
               )

            ^ (pp.Keyword("ALTER")('opcode')
               + p_coordsLabel + ","
               + p_opcLabel + ","
               + p_dungeonItemID
               )

            ^ pp.Keyword("REDRAW")('opcode')

            ^ pp.Keyword("CLRENC")('opcode')

            ^ pp.Keyword("GOENC")('opcode')+p_winOpcLabel+","+p_loseOpcLabel

            ^ (pp.Keyword("ADDENC")('opcode')
               + p_monsterID('tMonsterID') + ","
               + p_monsterLevel('tMonsterLevel') + ","
               + p_monsterID('tMonsterCount') + ","
               + p_monsterLevel('tMonsterRow')
               )

            ^ (pp.Keyword("ENTER_W")('opcode')
               + p_mapID + ","
               + p_xValue + ","
               + p_yValue)

            # ---------- meta commands -----------

            ^ (pp.Keyword("defc")('metaCmd')
               + p_coordsLabel+":"
               + p_xValue
               + ","+p_yValue + pp.Optional("-"+p_x2Value + ","+p_y2Value)
               )

            ^ (pp.Keyword("includemap")('metaCmd')
               + p_quoted_string('tMapName'))

            ^ pp.Keyword("---")('metaCmd')

            ^ pp.Keyword("$")('metaCmd')+p_msgLabel+pp.Suppress(",")+p_quoted_string('tMessage')

            ^ pp.Keyword("&")('metaCmd')+p_quoted_string('tMsgFile')+pp.Suppress(",")+p_quoted_string('tFileMessage')

        )

        p_query = p_keywords  # +pp.ZeroOrMore(pp.delimitedList(value))
        p_table = []

        for lineTupel in codeLines:
            lineNum = lineTupel[0]
            line = lineTupel[1]
            try:
                a = p_query.parseString(line)
            except pp.ParseException as e:
                print("parse error at line "+str(lineNum)+":")
                print(e)
                exit(-1)
            p_table.append((lineNum, a))
        # print("========== p_table ==========")
        # pp.pprint.pprint(p_table)
        # print("======== p_table end ========")
        self.buildStringsAndCoords(p_table)
        return self.buildOpcodes(p_table)

    def compile(self, srcFile):

        print("Reading "+sys.argv[1])
        infile = open(srcFilename, "r")
        self.rawLines = infile.readlines()
        srcLines = self.trim(self.rawLines)
        infile.close()

        codeLines = self.scanAndTrimLabels(srcLines)
        self.gOpcodes = self.parseScript(codeLines)

        # build a list of [labeled map position] -> label line numbers
        # so that a map element doesn't need the absolute address of its opcode
        # but an INDEX in a jump table

        print ("building map coords table...")
        jumpTableIdx = 0
        jumpTable = []
        jumpTableMapping = {}
        for i in self.gCoordsMapping:
            label = i+":"
            labelLineNumber = self.gLabels.get(label)
            opcAddress = 0
            if not (labelLineNumber is None):
                opcAddress = self.gLinePosMapping[labelLineNumber]
            jumpTable.append(opcAddress)
            jumpTableMapping[label] = jumpTableIdx
            print ("idx "+str(jumpTableIdx)+" : opcAdr "+str(opcAddress)+" ("+label+")")
            jumpTableIdx += 1

        print("Connecting map positions...")
        for i in self.gCoordsMapping:
            label = i+":"
            labelLineNumber = self.gLabels.get(label)
            opcodeNumber = 0
            if not (labelLineNumber is None):
                opcodeNumber = self.gLinePosMapping[labelLineNumber]
                coords = self.gCoordsMapping.get(i)
                for x in range(coords[0], coords[2]+1):
                    for y in range(coords[1], coords[3]+1):
                        # self.map[x][y].startOpcodeIndex = opcodeNumber
                        self.map[x][y].startOpcodeIndex = jumpTableMapping[label]
                        # print (x,y,jumpTableMapping[label])

        # pp.pprint.pprint (self.opcodeBytes())
        # pp.pprint.pprint (self.feelsBytes())
        # pp.pprint.pprint (self.mapBytes())


##################
#      main      #
##################

mc = mapCompiler()

print("\nDragonRock map compiler v0.1 alpha")
print("Written by Stephan Kleinert\n")

if len(sys.argv) < 3:
    print("usage: "+sys.argv[0]+" infile outfile")
    exit(-1)

srcFilename = sys.argv[1]
destFilename = sys.argv[2]
mc.compile(srcFilename)
print("exporting...")
mc.export(destFilename)
print("done.")
