#!/bin/sh

set -e

if [ ! -f "disc/drock.d81" ]; then
  mkdir -p disc
  c1541 -format drock,sk d81 disc/drock.d81
fi

/bin/sh tools/buildResources.sh

cat cbm/wrapper.prg bin/drmain.c64 > bin/main

c1541 <<EOF
attach disc/drock.d81
delete loader
delete autoboot.c65
delete main
delete city
delete dungeon
delete encounter
delete charset
delete map*
delete out*
delete spr*
delete fmsg*
write cbm/autoboot.c65
write bin/main main
write bin/drmain.c64.1 dungeon
write bin/drmain.c64.2 city
write bin/drmain.c64.3 encounter
write bin/drcharset charset
EOF

c1541 disc/drock.d81 -delete fmsg

for filename in graphics/*.pbm; do
  c1541 disc/drock.d81 -delete $(basename $filename)
  c1541 disc/drock.d81 -write $filename
done

for filename in mapdata/fmsg*; do 
  c1541 disc/drock.d81 -write $filename
done

for filename in mapdata/map*; do
  c1541 disc/drock.d81 -write $filename
done

for filename in mapdata/out*; do
  c1541 disc/drock.d81 -write $filename
done
  
