#!/bin/sh
PARAM=$1
asl $PARAM.asm -A -a -L -u -E !1 -x
p2bin $PARAM.p $PARAM.prg -r '$-$' -l 0 -k
