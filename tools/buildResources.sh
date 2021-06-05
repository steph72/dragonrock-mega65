#!/bin/sh

set -e

tools/buildCharset.sh
tools/buildMaps.sh

python3 tools/genItems.py gamedata-src/items.yaml  gamedata/items
python3 tools/genMonsters.py gamedata-src/monsters.yaml  gamedata/monsters

