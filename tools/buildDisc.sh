#!/bin/sh

python3 tools/mc.py mapsrc/library.drs mapdata/library.d

if [ ! -f "disc/drock.d64" ]; then
  mkdir -p disc
  c1541 -format drock,sk d64 disc/drock.d64
fi

c1541 <<EOF
attach disc/drock.d64
delete main
delete city
delete dungeon
delete charset
delete map*
write bin/drmain.plus4   main
write bin/drmain.plus4.1 dungeon
write bin/drmain.plus4.2 city
write bin/charset1 charset
write mapdata/library.d map0
EOF

c1541 disc/drock.d64 -delete fmsg*
c1541 disc/drock.d64 -write mapdata/fmsg*
