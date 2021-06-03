#!/bin/sh

set -e

tools/buildCharset.sh
tools/buildMaps.sh

python3 tools/genItems.py mapsrc/items.csv  gamedata/items
