#!/bin/sh

set -e

rm -rf gamedata/*

tools/buildCharset.sh
tools/buildMaps.sh

python3 tools/genItems.py gamedata-src/items.yaml gamedata/items
python3 tools/genMonsters.py gamedata-src/monsters.yaml gamedata

# copy ui images with palette
for filename in images-src/ui/*.png; do
  python3 tools/png2dbm.py -v $filename gamedata/$(basename $filename .png).dbm
done

# copy city images and shift palette
for filename in images-src/artwork/*.png; do
  python3 tools/png2dbm.py -vr $filename gamedata/$(basename $filename .png).dbm
done