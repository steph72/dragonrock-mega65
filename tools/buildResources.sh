#!/bin/sh

set -e

rm gamedata/*

tools/buildCharset.sh
tools/buildMaps.sh

python3 tools/genItems.py gamedata-src/items.yaml  gamedata/items
python3 tools/genMonsters.py gamedata-src/monsters.yaml  gamedata/monsters

for filename in images-src/*.png; do
  python3 tools/png2dbm.py -vr $filename gamedata/$(basename $filename .png).dbm
done
