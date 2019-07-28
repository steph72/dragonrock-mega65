#!/usr/bin/python3

import sys
import pyparsing as pp

gLabels = {}
gStringMapping = {}

gOpcodes = []
gStrings = []

class mapElement:
    pass


def trim(lines):
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


def scanAndTrimLabels(scrLines):
    returnLines = []
    print("Scanning labels")
    currentLabels = []
    for lineTupel in srcLines:
        lineNum = lineTupel[0]
        line = lineTupel[1]
        if line.endswith(":"):
            if (gLabels.get(line)):
                print("error: duplicate label definition at line", lineNum)
                print("       (original definition was at line " +
                      str(gLabels.get(line))+")")
                exit(-1)
            currentLabels.append(line)
        else:
            returnLines.append((lineNum, line))
            for i in currentLabels:
                gLabels[i] = lineNum
            currentLabels = []
    return returnLines


def buildStrings(p_table):
    for i in p_table:
        line = i[0]
        src = i[1]
        if (src.metaCmd == "$"):
            gStringMapping[src.tMsgLabel] = len(gStrings)
            gStrings.append(src.tMessage)

def buildOpcodes(p_table):

    def checkString(aLabel,pline):
        if not gStringMapping.get(aLabel):
            print ("error: can't find string \""+aLabel+"\" at line "+str(pline.lineNum))
            exit(-1)

    # -------------- opcode factory --------------

    def opCreate_NOP(pline):
        return [0,0,0,0,0,0,0,0]

    def opCreate_NSTAT(pline):
        checkString(pline.tMsgLabel,pline)
        return [1,gStringMapping[pline.tMsgLabel],0,0,0,0,0,0]

    def opCreate_DISP(pline):
        checkString(pline.tMsgLabel,pline)
        opc = [2,gStringMapping[pline.tMsgLabel],0,0,0,0,0,0]
        if (pline.clrFlag==True):
            opc[2] = 1
        return opc

    def opCreate_WKEY(pline):
        pass

    def opCreate_YESNO(pline):
        opc = [4,0,0,0,0,0,0,0]
        if (pline.tTrueOpcLabel):
            opc[1] = "__DRLABEL__"+pline.tTrueOpcLabel
        if (pline.tFalseOpcLabel):
            opc[2] = "__DRLABEL__"+pline.tFalseOpcLabel
        return opc
    
    def opCreate_IFREG(pline):
        pass

    def opCreate_IFPOS(pline):
        pass

    def opCreate_IADD(pline):
        pass
    
    def opCreate_ALTER(pline):
        pass

    def opCreate_REDRAW(pline):
        pass

    def opCreate_EXIT(pline):
        pass

    lastOpcodeIndex = 0
    lastOpcode = []

    for i in p_table:
        lineNum = i[0]
        src = i[1]
        if src.metaCmd=="---":
            print("RESET")
            lastOpcode = []
        if not src.opcode:
            continue

        src.lineNum = lineNum
        opCreateFunc = "opCreate_"+src.opcode     # construct building function name
        newOpcode = locals()[opCreateFunc](src)   # ...and call it.
        gOpcodes.append((lineNum,newOpcode))

        newOpcodeIndex = len(gOpcodes)-1 

        if (lastOpcode):
            lastOpcode[7] = newOpcodeIndex        # link last opcode to current...

        lastOpcode = newOpcode                    # ...and save this opcode


def parseScript(codeLines):

    p_numeric_value = pp.pyparsing_common.number()
    p_quoted_string = pp.QuotedString('"')
    p_TRUE = pp.CaselessKeyword("true").setParseAction(lambda tokens: True)
    p_FALSE = pp.CaselessKeyword("false").setParseAction(lambda tokens: False)

    p_boolean_literal = p_TRUE | p_FALSE

    value = p_numeric_value | p_quoted_string | p_boolean_literal | pp.Word(pp.alphanums)

    p_msgLabel      = pp.Word(pp.alphanums)('tMsgLabel')
    p_trueOpcLabel  = pp.Word(pp.alphanums)('tTrueOpcLabel')
    p_falseOpcLabel = pp.Word(pp.alphanums)('tFalseOpcLabel')

    p_keywords = (

        # opcodes
        pp.Keyword("NOP")('opcode')
        ^ pp.Keyword("NSTAT")('opcode')+p_msgLabel
        ^ pp.Keyword("DISP")('opcode')+p_msgLabel+pp.Optional(","+p_boolean_literal('clrFlag'))
        ^ pp.Keyword("WKEY")('opcode')
        ^ pp.Keyword("YESNO")('opcode')+p_trueOpcLabel+pp.Optional(","+p_falseOpcLabel)
        ^ pp.Keyword("IFPOS")('opcode')
        ^ pp.Keyword("REDRAW")('opcode')
        ^ pp.Keyword("EXIT")('opcode')

        # meta commands
        ^ pp.Keyword("includemap")('metaCmd')
        ^ pp.Keyword("---")('metaCmd')
        ^ pp.Keyword("$")('metaCmd')+p_msgLabel+pp.Suppress(",")+p_quoted_string('tMessage')
    )

    p_query = p_keywords #+pp.ZeroOrMore(pp.delimitedList(value))
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
        p_table.append((lineNum,a))
    print("========== p_table ==========")
    pp.pprint.pprint(p_table)
    print("======== p_table end ========")
    buildStrings(p_table)
    buildOpcodes(p_table)


##################
#      main      #
##################

print("\nDragonRock map compiler v0.1 alpha")
print("Written by Stephan Kleinert\n")

if len(sys.argv) < 2:
    print("usage: "+sys.argv[0]+" infile [outfile]")
    exit(-1)

srcFilename = sys.argv[1]
print("Reading "+sys.argv[1])
infile = open(srcFilename, "r")
srcLines = trim(infile.readlines())
codeLines = scanAndTrimLabels(srcLines)
parseScript(codeLines)

print("==== STRINGS ====\n", gStringMapping, "\n-------->\n",gStrings)
print("==== OPCODES ====")
pp.pprint.pprint(gOpcodes)