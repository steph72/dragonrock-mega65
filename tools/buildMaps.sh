#! /bin/sh

set -e

python3 tools/mc.py mapsrc/library.drs mapdata/map000
python3 tools/mc.py mapsrc/outdoor33.drs mapdata/out033
