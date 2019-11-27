#!/bin/sh

if [ ! -f "disc/drock.d64" ]; then
  mkdir -p disc
  c1541 -format drock,sk d64 disc/drock.d64
fi

tools/buildCharset.sh
tools/buildSprites.sh

python3 tools/mc.py mapsrc/library.drs mapdata/library.d

c1541 disc/drock.d64 -delete fmsg*

for filename in mapdata/fmsg*; do 
  c1541 disc/drock.d64 -write $filename
done

