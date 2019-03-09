#!/bin/sh
c1541 <<EOF
format drock,sk d64 disc/drock.d64
attach disc/drock.d64
write bin/drmain.c128 drmain
EOF
