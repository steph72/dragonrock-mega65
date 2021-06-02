#!/usr/bin/python3

import sys
import pickle
import csv

types = ["it_armor",
         "it_shield",
         "it_weapon",
         "it_missile",
         "it_potion",
         "it_scroll",
         "it_special"]


def checkFormat(aRow):
    if len(aRow) != 7:
        return False, "7 columns expected but got "+str(len(aRow))
    type = aRow[2].strip()
    if not type in types:
        return False, "Unknown type '"+type+"'"
    return True, ""


def read(aFile):
    itemRows = []
    processedRow = 0
    with open(aFile, newline='') as csvfile:
        confreader = csv.reader(csvfile, delimiter=',', quotechar='\"')
        for row in confreader:
            processedRow += 1
            if len(row) > 0:
                if row[0][0] != "#":
                    checkResult, msg = checkFormat(row)
                    if checkResult == False:
                        print("Error:\n"+msg+" at line "+str(processedRow))
                        exit(1)
                    itemRows.append(row)
                    # print(', '.join(row))
                else:
                    pass
    return itemRows


def buildDescriptions(src):
    offsets = []
    destbytes = bytearray()
    currentOffset = 0
    for i in src:
        offsets.append(currentOffset)
        commobytes = bytearray()
        unixbytes = bytearray()
        i = i.replace("&&nl&&", "\n")
        unixbytes.extend(map(ord, i.swapcase()))
        for p in unixbytes:  # lf -> crlf
            currentOffset += 1
            if (p == 10):
                commobytes.append(13)
            else:
                commobytes.append(p)
        currentOffset += 1
        commobytes.append(0)
        destbytes.extend(commobytes)
    return destbytes, offsets


def rowsToData(srcRows):
    descriptions = []
    items = {}
    for i in srcRows:
        anItem = {}
        itemID = int(i[0], 0)
        if itemID in items:
            print("Error: Duplicate item ID "+i[0])
            exit(1)
        desc = i[1].strip().strip("\"")
        if not desc in descriptions:
            descriptions.append(desc)
        anItem["description"] = descriptions.index(desc)
        anItem["type"] = types.index(i[2].strip())
        anItem["val1"] = int(i[3])
        anItem["val2"] = int(i[4])
        anItem["val3"] = int(i[5])
        anItem["xp"] = int(i[6])
        items[itemID] = anItem

    descbytes, offsets = buildDescriptions(descriptions)
    print(offsets)
    print(descbytes)

    return 0


if len(sys.argv) < 3:
    print("usage: "+sys.argv[0]+" infile outfile")
    exit(1)

srcFilename = sys.argv[1]
destFilename = sys.argv[2]

itemRows = read(srcFilename)
itemData = rowsToData(itemRows)