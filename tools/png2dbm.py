#!/usr/bin/python3

#######################################################################
# png2dbm version 1.0                                                 #
# written by stephan kleinert @ hundehaus im reinhardswald, june 2020 #
# with very special thanks to frau k., buba k. and candor k.!         #
#######################################################################

import sys
import png

gVerbose = False
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

infile, outfile, gReserve, gVerbose = parseArgs()

vprint("### png2dbm v"+gVersion+" ###")
vprint("reading", infile)
pngReader = png.Reader(filename=infile)
pngData = pngReader.read()
pngInfo = pngData[3]

try:
    palette = pngInfo["palette"]
except e:
    print(e)
    print("error: infile has no palette")
    exit(1)

vicii_palette = []

if gReserve:
    if len(palette) > 240:
        print("error: can't reserve system palette because source PNG "
              "has >240 palette entries.")
        exit(2)

    # add placeholders for system colours
    for i in range(16):
        vicii_palette.append((0, 0, 0))

vprint("swapping nybbles for",len(palette),"palette entries")
# swap nyybles in palette
for i in palette:
    r = nybswap(i[0])
    g = nybswap(i[1])
    b = nybswap(i[2])
    vicii_palette.append((r, g, b))

rows = list(pngData[2])

for currentRow in rows:
    for currentColumn in currentRow:
        if gReserve:
            currentColumn += 16


print (len(a))
print (len(a[0]))
# print(a[0])
