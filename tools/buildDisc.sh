#!/bin/sh

tools/buildCharset.sh

python3 tools/mc.py mapsrc/library.drs mapdata/library.d

# printf "00" | cat - graphics/dr_charset.bin > bin/charset

if [ ! -f "disc/drock.d64" ]; then
  mkdir -p disc
  c1541 -format drock,sk d64 disc/drock.d64
fi

c1541 <<EOF
attach disc/drock.d64
delete loader
delete sjload
delete main
delete city
delete dungeon
delete encounter
delete charset
delete map*
write bin/drmain.plus4   main
write bin/drmain.plus4.1 dungeon
write bin/drmain.plus4.2 city
write bin/drmain.plus4.3 encounter
write bin/drcharset charset
write mapdata/library.d mapa
write cbm/loader sd2iecload
write cbm/sjload sjload
EOF

c1541 disc/drock.d64 -delete fmsg*
c1541 disc/drock.d64 -delete spr*
c1541 disc/drock.d64 -write mapdata/fmsg*
c1541 disc/drock.d64 -write graphics/spr*
