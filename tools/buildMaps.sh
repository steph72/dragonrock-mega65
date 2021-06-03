#! /bin/sh

set -e

python3 tools/mc.py maps/library.drs      gamedata/map000
python3 tools/mc.py maps/terrorShack.drs  gamedata/map001
python3 tools/mc.py maps/outdoor33.drs    gamedata/out033
