#!/bin/sh

tools/buildCharset.sh
tools/buildSprites.sh

python3 tools/mc.py mapsrc/library.drs mapdata/library.d

