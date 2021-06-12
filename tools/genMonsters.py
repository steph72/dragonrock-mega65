#!/usr/bin/python3

import sys
import pickle
import csv
import yaml

monsterType = {
    "mt_humanoid": 1,
    "mt_animal": 2,
    "mt_magical": 4,
    "mt_unique": 128
}

attackTypes = {
    "at_fists": 1,
    "at_weapon": 2,
    "at_fire": 4,
    "at_ice": 8,
    "at_claws": 16,
    "at_drain": 32,
    "at_breath": 64,
    "at_reserved": 128
}

def read(aFilename):
    with open(aFilename, 'r') as stream:
        try:
            monsterData = yaml.safe_load(stream)
        except yaml.YAMLError as exc:
            print(exc)
            exit(127)
    return monsterData


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


def reduce(aMonster):
    global ids
    global names
    i = aMonster
    monsterID = i["id"]
    if monsterID in ids:
        print("Error: Duplicate item ID in", i)
        exit(1)

    names.append(i["name"])
    i["name"] = len(names)-1

    if "pluralName" in i:
        names.append(i["pluralName"])
        i["pluralName"] = len(names)-1

    atype = []
    for at in i["attackTypes"]:
        atype.append(attackTypes[at])
    
    i["attackTypes"] = atype

    mtype = 0
    for mt in i["monsterType"]:
        mtype += monsterType[mt]
    i["monsterType"] = mtype

    while len(i["attackTypes"])<4:
        i["attackTypes"].append(0)

    while len(i["minDamage"])<4:
        i["minDamage"].append(0)

    while len(i["maxDamage"])<4:
        i["maxDamage"].append(0)

    while len(i["hitModifier"])<4:
        i["hitModifier"].append(0)
        
    print (i)
    return i


def toMonsters(data):
    global ids
    global names

    ids = []
    names = []
    monsters = []

    monsters.extend(map(reduce, data))
    descbytes, offsets = buildDescriptions(names)
    # print(descbytes, offsets)

    # replace desc index with offset
    # assuming one item definition = 10 bytes

    itemMarker = "DRMONST0"
    monsterRecordLength = 19   # IMPORTANT: Keep in sync with C struct!!

    stringsBase = len(itemMarker)+(len(monsters)*monsterRecordLength)
    # print("Strings base is", hex(stringsBase))

    outbytes = bytearray()
    outbytes.extend(map(ord, itemMarker))

    for i in monsters:
        i["name"] = stringsBase + offsets[i["name"]]
        if "pluralName" in i:
            i["pluralName"] = stringsBase + offsets[i["pluralName"]]
        else:
            i["pluralName"] = 0

        # print(i)
        outbytes.append(i["id"] % 256)              #  0 : id
        outbytes.append(i["id"]//256)               #  1 :
        outbytes.append(i["defaultLevel"])          #  2 : defaultLevel
        outbytes.append(i["spriteID"])              #  3 : spriteID
        outbytes.append(i["monsterType"])           #  4 : monster type
        outbytes.append(i["name"] % 256)            #  5 : monster name
        outbytes.append(i["name"]//256)             #  6 :
        outbytes.append(i["pluralName"] % 256)      #  7 : plural name
        outbytes.append(i["pluralName"]//256)       #  8 :
        if i["AC"] < 0:
            outbytes.append(i["AC"]+256)            #  9 : armor class
        else:
            outbytes.append(i["AC"])
        for c in range(4):
            outbytes.append(i["minDamage"][i])

        outbytes.append(i["hitDice"])               # 10 : hit dice
        outbytes.append(i["hitPoints"])             # 11 : hit points per level
        outbytes.append(i["magPoints"])             # 12 : mag points per level
        outbytes.append(i["numAttacks"])            # 13 : num attacks
        if i["courageMod"] < 0:
            outbytes.append(i["courageMod"]+256)    # 14 : courage modifier
        else:
            outbytes.append(i["courageMod"])
        if i["hitMod"] < 0:
            outbytes.append(i["hitMod"]+256)        # 15 : hit modifier
        else:
            outbytes.append(i["hitMod"])
        outbytes.append(i["attackTypes"])           # 16 : attack types
        outbytes.append(i["xpValue"] % 256)         # 17 : plural name
        outbytes.append(i["xpValue"]//256)          # 18 :

    outbytes.extend(descbytes)

    # print(offsets)
    # print(descbytes)
    # print(outbytes)

    return outbytes


print("DragonRock monster builder v0.1, (w) Stephan Kleinert, 2021/06")


if len(sys.argv) < 3:
    print("usage: "+sys.argv[0]+" infile outfile")
    exit(1)

srcFilename = sys.argv[1]
destFilename = sys.argv[2]

monsterDict = read(srcFilename)
if "monsters" in monsterDict:
    monsterData = toMonsters(monsterDict["monsters"])
else:
    print("?no monster node found in configuration file")
    exit(1)

outfile = open(destFilename, "wb")
outfile.write(monsterData)
print("Monster file written successfully.")
outfile.close()
