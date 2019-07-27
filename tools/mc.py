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
                #end reading multiline string
                i = i.replace("\n","")  # remove last newline
                currentMultilineString += i
                currentMultilineString = currentMultilineString.replace("\\n","&&nl&&")
                currentMultilineString = currentMultilineString.replace("\"\"\"","\"")
                currentMultilineString = currentMultilineString.replace("\n","&&nl&&")
                out.append((lineNum,currentMultilineString))
                isReadingMultilineString = False
                # print ("====>"+currentMultilineString+"<====")
                continue
            else:
                #start reading multiline string
                currentMultilineString = ""
                i = i.replace("\n","")  # remove first newline
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

def scanLabels(scrLines):
    print("Scanning labels")
    currentLabels = []
    for lineTupel in srcLines:
        lineNum = lineTupel[0]
        line = lineTupel[1]
        if line.endswith(":"):
            if (gLabels.get(line)):
                print ("error: duplicate label definition at line",lineNum)
                print ("       (original definition was at line "+str(gLabels.get(line))+")")
                exit(-1)
            currentLabels.append(line)
        else:
            for i in currentLabels:
                gLabels[i] = lineNum
            currentLabels=[]
    print(gLabels)

def parseScript(srcLines):

    p_keywords = pp.Keyword("NOP") ^             \
                 pp.Keyword("NSTAT") ^           \
                 pp.Keyword("DISP") ^            \
                 pp.Keyword("WKEY") ^            \
                 pp.Keyword("IFPOS") ^           \
                 pp.Keyword("includemap") ^      \
                 ("$")

    p_numeric_value = pp.pyparsing_common.number()
    p_quoted_string = pp.quotedString()
    p_TRUE = pp.CaselessKeyword("true").setParseAction(lambda tokens: True)
    p_FALSE = pp.CaselessKeyword("false").setParseAction(lambda tokens: False)

    p_boolean_literal = p_TRUE | p_FALSE

    value = p_numeric_value | p_quoted_string | p_boolean_literal | pp.Word(pp.alphanums)

    p_query = p_keywords("command")+pp.ZeroOrMore(pp.delimitedList(value))

    for lineTupel in srcLines:
        lineNum = lineTupel[0]
        line = lineTupel[1]
        if line.endswith(":"):
            continue
        a = []
        try:
            a = p_query.parseString(line)
        except pp.ParseException as e:
            print ("parse error at line "+str(lineNum)+":")
            print (e)
            exit (-1)
        print (a)



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
for i in srcLines:
    print (i)

scanLabels(srcLines)
parseScript(srcLines)