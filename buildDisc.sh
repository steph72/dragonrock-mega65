#!/bin/sh

if [ ! -f "disc/drock.d64" ]; then
  mkdir -p disc
  c1541 -format drock,sk d64 disc/drock.d64
fi

c1541 <<EOF
attach disc/drock.d64
delete main
delete city
delete dungeon
delete outdoor
delete em
delete map0
write bin/drmain.plus4   main
write bin/drmain.plus4.1 dungeon
write bin/drmain.plus4.2 city
write mapdata/testmap.drm map0
EOF
