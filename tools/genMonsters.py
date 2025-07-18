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

spellClass = {
    "sc_priest": 1,
    "sc_necromancer": 2,
    "sc_battlemage": 4
}

attackTypes = {
    "at_fists": 1,
    "at_weapon": 2,
    "at_fire": 4,
    "at_ice": 8,
    "at_claws": 16,
    "at_drain": 32,
    "at_breath": 64,
    "at_spell": 128
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


def reduceCity(aCity):
    global ids
    global names
    i = aCity
    cityID = i["id"]
    if cityID in ids:
        print("Error: duplicate city ID in", i)
        exit(1)

    names.append(i["cityName"])
    i["nameIdx"] = len(names)-1
    names.append(i["innName"])
    i["innNameIdx"] = len(names)-1
    names.append(i["armoryOwnerName"])
    i["armoryOwnerNameIdx"] = len(names)-1
    return i


def reduceMonster(aMonster):
    global ids
    global names
    i = aMonster
    monsterID = i["id"]
    if monsterID in ids:
        print("Error: Duplicate monster ID in", i)
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

    sclass = 0
    if "spellClass" in i:
        for sc in i["spellClass"]:
            sclass += spellClass[sc]
    i["spellClass"] = sclass

    while len(i["attackTypes"]) < 4:
        i["attackTypes"].append(0)

    while len(i["minDamage"]) < 4:
        i["minDamage"].append(0)

    while len(i["maxDamage"]) < 4:
        i["maxDamage"].append(0)

    while len(i["hitModifier"]) < 4:
        i["hitModifier"].append(0)

    # print (i)
    return i


def reduceSpells(aSpell):
    global ids
    global names
    i = aSpell

    spellID = i["id"]
    if spellID in ids:
        print("Error: Duplicate spell ID in", i)
        exit(1)
        

def toSpells(data):
    global ids
    global names
    ids = []
    names = []
    spells = []

    spells.extend(map(reduceSpells,data))



def toCities(data):
    global ids
    global names
    ids = []
    names = []
    cities = []

    cities.extend(map(reduceCity, data))
    descbytes, offsets = buildDescriptions(names)

    cityMarker = "DRCITY00"
    outbytes = bytearray()
    cityRecordLength = 10    # keep in sync with monsterDef struct!
    stringsBase = len(cityMarker)+(len(cities)*cityRecordLength)

    outbytes = bytearray()

    outbytes.extend(map(ord, cityMarker))

    for i in cities:
        cityNameOffset = stringsBase+offsets[i["nameIdx"]]
        innNameOffset = stringsBase+offsets[i["innNameIdx"]]
        armoryOwnerNameOffset = stringsBase+offsets[i["armoryOwnerNameIdx"]]
        outbytes.append(i["id"])        # 0
        outbytes.append(i["mapNr"])     # 1
        outbytes.append(i["x"])         # 2
        outbytes.append(i["y"])         # 3
        outbytes.append(cityNameOffset % 256) #4   
        outbytes.append(cityNameOffset//256)  #5
        outbytes.append(innNameOffset % 256)  #6
        outbytes.append(innNameOffset//256)   #7
        outbytes.append(armoryOwnerNameOffset % 256) #8
        outbytes.append(armoryOwnerNameOffset//256)  #9
    outbytes.extend(descbytes)

    print(outbytes)
    return outbytes


def toMonsters(data):
    global ids
    global names

    ids = []
    names = []
    monsters = []

    monsters.extend(map(reduceMonster, data))
    descbytes, offsets = buildDescriptions(names)
    # print(descbytes, offsets)

    # replace desc index with offset

    itemMarker = "DRMONST0"
    monsterRecordLength = 32   # IMPORTANT: Keep in sync with C struct!!

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
        outbytes.append(i["id"] % 256)  # 0-1 : id
        outbytes.append(i["id"]//256)
        outbytes.append(i["defaultLevel"])  # 2 : defaultLevel
        outbytes.append(i["spriteID"])  # 3 : spriteID
        outbytes.append(i["monsterType"])  # 4 : monster type
        outbytes.append(i["name"] % 256)  # 5-6 : monster name
        outbytes.append(i["name"]//256)
        outbytes.append(i["pluralName"] % 256)  # 7-8 : plural name
        outbytes.append(i["pluralName"]//256)
        if i["AC"] < 0:
            outbytes.append(i["AC"]+256)  # 9 : armor class
        else:
            outbytes.append(i["AC"])
        for c in i["attackTypes"]:                  # 10-13: attackTypes
            outbytes.append(c)
        for c in i["minDamage"]:                    # 14-17: minDamage
            outbytes.append(c)
        for c in i["maxDamage"]:                    # 18-21: maxDamage
            outbytes.append(c)
        for c in i["hitModifier"]:                  # 22-25: hitModifier
            if (c < 0):
                outbytes.append(c+256)
            else:
                outbytes.append(c)
        outbytes.append(i["hitPoints"])             # 26 : hit points per level
        outbytes.append(i["magPoints"])             # 27 : mag points per level
        if i["courageMod"] < 0:
            outbytes.append(i["courageMod"]+256)    # 28 : courage modifier
        else:
            outbytes.append(i["courageMod"])
        outbytes.append(i["spellClass"])            # 29 : spell class

        outbytes.append(i["xpValue"] % 256)         # 30-31 : xp value
        outbytes.append(i["xpValue"]//256)

    outbytes.extend(descbytes)
    return outbytes


print("DragonRock gamedata builder v0.1, (w) Stephan Kleinert, 2021/06")

if len(sys.argv) < 3:
    print("usage: "+sys.argv[0]+" infile configDir")
    exit(1)

srcFilename = sys.argv[1]
configDir = sys.argv[2]


configDict = read(srcFilename)
if "monsters" in configDict:
    monsterData = toMonsters(configDict["monsters"])
else:
    print("?no monster node found in configuration file")
    exit(1)

if "cities" in configDict:
    cityData = toCities(configDict["cities"])
else:
    print("?no city node found in configuration file")
    exit(1)

if "spells" in configDict:
    spellData = toSpells(configDict["spells"])
else:
#    print("?no spells node found in configuration file")
#    exit(1)
    pass
#TODO

outfile = open(configDir+"/monsters", "wb")
outfile.write(monsterData)
print("Monster file written successfully.")
outfile.close()

outfile = open(configDir+"/cities","wb")
outfile.write(cityData)
print("City file written successfully.")
outfile.close()