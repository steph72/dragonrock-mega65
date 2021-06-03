#!/usr/bin/python3

import sys
import pickle
import csv
import yaml

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
    type = aRow[2]
    if not type in types:
        return False, "Unknown type '"+type+"'"
    return True, ""


def read(aFilename):
    with open(aFilename, 'r') as stream:
        try:
            itemRows = yaml.safe_load(stream)
        except yaml.YAMLError as exc:
            print(exc)
    return itemRows


def buildDescriptions(src):
    offsets = []
    destbytes = bytearray()
    startMarker = "STRINGS*"
    destbytes.extend(map(ord, startMarker))
    currentOffset = len(startMarker)
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
        res,err = checkFormat(i)
        if (res==False):
            print (err+" in",i)
            exit(1)
        anItem = {}
        itemID = i[0]
        if itemID in items:
            print("Error: Duplicate item ID "+i[0])
            exit(1)
        desc = i[1]
        if not desc in descriptions:
            descriptions.append(desc)
        anItem["id"] = itemID
        anItem["descriptionIndex"] = descriptions.index(desc)
        anItem["type"] = types.index(i[2])
        anItem["val1"] = int(i[3])
        anItem["val2"] = int(i[4])
        anItem["val3"] = int(i[5])
        anItem["xp"] = int(i[6])
        items[itemID] = anItem

    descbytes, offsets = buildDescriptions(descriptions)

    # replace desc index with offset
    # assuming one item definition = 10 bytes

    itemMarker = "DRITEMS0"

    stringsBase = len(itemMarker)+(len(items)*10)
    print("Strings base is", hex(stringsBase))

    outbytes = bytearray()
    outbytes.extend(map(ord, itemMarker))

    for i in items:
        theItem = items[i]
        theItem["descOffset"] = stringsBase + \
            offsets[theItem["descriptionIndex"]]
        outbytes.append(theItem["id"] % 256)            # 0
        outbytes.append(theItem["id"]//256)
        outbytes.append(theItem["descOffset"] % 256)    # 2
        outbytes.append(theItem["descOffset"]//256)
        outbytes.append(theItem["type"])                # 4
        outbytes.append(theItem["val1"])                # 5
        outbytes.append(theItem["val2"])                # 6
        outbytes.append(theItem["val3"])                # 7
        outbytes.append(theItem["xp"] % 256)            # 8
        outbytes.append(theItem["xp"]//256)
    outbytes.extend(descbytes)

    # print(offsets)
    # print(descbytes)
    # print(outbytes)

    return outbytes

print("DragonRock item builder v0.1, (w) Stephan Kleinert, 2021/06")

if len(sys.argv) < 3:
    print("usage: "+sys.argv[0]+" infile outfile")
    exit(1)

srcFilename = sys.argv[1]
destFilename = sys.argv[2]

itemRows = read(srcFilename)
itemData = rowsToData(itemRows)

outfile = open(destFilename, "wb")
outfile.write(itemData)
outfile.close()
