#!/usr/bin/python3

#######################################################################
# png2dbm version 1.0                                                 #
# written by stephan kleinert @ hundehaus im reinhardswald, june 2020 #
# with very special thanks to frau k., buba k. and candor k.!         #
#######################################################################

import sys
import png
import io

gVerbose = False
gReserve = False
gVersion = "1.0"


def showUsage():
    print("usage: "+sys.argv[0]+" [-rv] infile outfile")
    print("convert PNG to MEGA65 DBM file")
    print("options: -r  reserve system palette entries")
    print("         -v  verbose output")
    exit(0)


def nybswap(i):
    lownyb = i % 16
    hinyb = i // 16
    out = (lownyb*16)+hinyb
    return out


def parseArgs():
    args = sys.argv.copy()
    args.remove(args[0])
    fReserve = False
    fVerbose = False
    fileargcount = 0

    for arg in args:
        if arg[0:1] == "-":
            opts = arg[1:]
            for opt in opts:
                if opt == "r":
                    fReserve = True
                elif opt == "v":
                    fVerbose = True
                else:
                    print("Unknown option", opt)
                    showUsage()
        else:
            fileargcount += 1
            if fileargcount == 1:
                infile = arg
            elif fileargcount == 2:
                outfile = arg
            else:
                print("too many arguments")
                showUsage()
    return infile, outfile, fReserve, fVerbose


def vprint(*values):
    if gVerbose:
        print(*values)


def pngRowsToM65Rows(pngRows):
    pngX = 0
    pngY = 0
    height = len(pngRows)
    width = len(pngRows[0])
    columnCount = width//8
    rowCount = height//8
    vprint("using", rowCount, "rows,",columnCount,"columns.")
    m65Rows = []
    for i in range(rowCount):
        aRow = []
        for b in range(columnCount):
            aChar = bytearray()
            for c in range(64):
                aChar.append(0)
            aRow.append(aChar)
        m65Rows.append(aRow)
    for currentRow in pngRows:
        pngX = 0
        for currentColumn in currentRow:
            if gReserve:
                currentColumn += 16
            m65X    = pngX//8
            m65Y    = pngY//8
            m65Byte = ((pngX%8)+(pngY*8))%64
            # print (pngX,pngY,"->",m65X,m65Y,m65Byte)
            m65Rows[m65Y][m65X][m65Byte] = currentColumn
            pngX += 1
        pngY += 1
    return m65Rows, rowCount, columnCount

####################### main program ########################

infile, outfile, gReserve, gVerbose = parseArgs()

vprint("### png2dbm v"+gVersion+" ###")
vprint("reading", infile)
pngReader = png.Reader(filename=infile)
pngData = pngReader.read()
pngInfo = pngData[3]

gWidth = pngInfo["size"][0]
gHeight = pngInfo["size"][1]

vprint("infile size is ", gWidth, "x", gHeight, "pixels")

try:
    palette = pngInfo["palette"]
except:
    print("error: infile has no palette")
    exit(1)

vic4_palette = []

if gReserve:
    if len(palette) > 240:
        print("error: can't reserve system palette because source PNG "
              "has >240 palette entries.")
        exit(2)

    # add placeholders for system colours
    for i in range(16):
        vic4_palette.append((0, 0, 0))

# swap nyybles in palette
vprint("swapping nybbles for", len(palette), "palette entries")
for i in palette:
    r = nybswap(i[0])
    g = nybswap(i[1])
    b = nybswap(i[2])
    vic4_palette.append((r, g, b))

rows = list(pngData[2])
m65Rows, numRows, numColumns = pngRowsToM65Rows(rows)
m65file = bytearray()

vprint("building outfile")
m65file.extend(map(ord,'dbmp'))         #   0-3 : identifier bytes for format
m65file.append(0x01)                    #     4 : version
m65file.append(numRows)                 #     5 : number of rows
m65file.append(numColumns)              #     6 : number of columns
m65file.append(gReserve)                #     7 : options byte (bit 0: system colours reserved)
m65file.append(len(vic4_palette))       #     8 : palette size
m65file.extend(map(ord,'pal'))          #  9-11 : pal header 

for entry in vic4_palette:
    m65file.extend(entry)

m65file.extend(map(ord,'img'))          #         bitmap data header

for currentRow in m65Rows:
    for currentColumn in currentRow:
            m65file.extend(currentColumn)

