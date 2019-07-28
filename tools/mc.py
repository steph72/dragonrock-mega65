#!/usr/bin/python3

import sys
import pyparsing as pp

gLabels = {}


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


def parseScript(codeLines):

    p_numeric_value = pp.pyparsing_common.number()
    p_quoted_string = pp.quotedString()
    p_TRUE = pp.CaselessKeyword("true").setParseAction(lambda tokens: True)
    p_FALSE = pp.CaselessKeyword("false").setParseAction(lambda tokens: False)

    p_boolean_literal = p_TRUE | p_FALSE

    value = p_numeric_value | p_quoted_string | p_boolean_literal | pp.Word(
        pp.alphanums)

    p_nstatMsgLabel = pp.Word(pp.alphanums)

    p_keywords = (

        # opcodes
        pp.Keyword("NOP")('opcode')
        
        ^ pp.Keyword("NSTAT")('opcode')+p_nstatMsgLabel('tMsgLabel')
        ^ pp.Keyword("DISP")('opcode')
        ^ pp.Keyword("WKEY")('opcode')
        ^ pp.Keyword("YESNO")('opcode')
        ^ pp.Keyword("IFPOS")('opcode')
        ^ pp.Keyword("EXIT")('opcode')

        # meta commands
        ^ pp.Keyword("includemap")('metaCmd')
        ^ pp.Keyword("$")(
            'metaCmd')+value('tMessageLabel')+pp.Suppress(",")+value('tMessage')
    )

    p_query = p_keywords+pp.ZeroOrMore(pp.delimitedList(value))
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
        p_table.append(a)
    pp.pprint.pprint(p_table)


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
