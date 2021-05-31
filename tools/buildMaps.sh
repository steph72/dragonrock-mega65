#! /bin/sh

set -e

python3 tools/mc.py mapsrc/library.drs      gamedata/map000
python3 tools/mc.py mapsrc/terrorShack.drs  gamedata/map001
python3 tools/mc.py mapsrc/outdoor33.drs    gamedata/out033
