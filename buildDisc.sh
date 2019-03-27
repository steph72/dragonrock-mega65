#!/bin/sh

if [ ! -f "disc/drock.d64" ]; then
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
write bin/drmain.c128 main
write bin/drmain.c128.1 city
write bin/drmain.c128.2 dungeon
write bin/drmain.c128.3 outdoor
write data/testmap map0
write drivers/c128-ram2.emd em 
EOF
