#!/bin/sh

set -e

if [ ! -f "disc/drock.d81" ]; then
  mkdir -p disc
  c1541 -format drock,sk d81 disc/drock.d81
fi

/bin/sh tools/buildResources.sh

cat cbm/wrapper.prg bin/drmain > bin/main

c1541 <<EOF
attach disc/drock.d81
delete loader*
delete tbnk*
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
delete items
delete monsters
write cbm/autoboot.c65
write cbm/loader0
write cbm/loader1
write cbm/tbnk4
write cbm/tbnk5
write bin/main
write bin/dungeon
write bin/encounter
write bin/city
write bin/drcharset charset
EOF

for filename in graphics/*.dbm; do
  c1541 disc/drock.d81 -delete $(basename $filename)
  c1541 disc/drock.d81 -write $filename
done

for filename in gamedata/*; do
  c1541 disc/drock.d81 -delete $(basename $filename)
  c1541 disc/drock.d81 -write $filename
done

#for filename in gamedata/map*; do
#  c1541 disc/drock.d81 -write $filename
#done

#for filename in gamedata/out*; do
#  c1541 disc/drock.d81 -write $filename
#done
  
