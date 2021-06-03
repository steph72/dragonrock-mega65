#!/bin/sh

set -e

tools/buildCharset.sh
tools/buildMaps.sh

python3 tools/genItems.py gamedata-src/items.csv  gamedata/items
