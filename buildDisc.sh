#!/bin/sh

if [ ! -f "disc/drock.d64" ]; then
  c1541 -format drock,sk d64 disc/drock.d64
fi

c1541 <<EOF
attach disc/drock.d64
delete drmain
write bin/drmain.c128 drmain
EOF
