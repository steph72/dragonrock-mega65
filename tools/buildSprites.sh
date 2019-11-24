#!/bin/sh

#create single sprite files (144 bytes) out of monstersX-charset.bin

cd bin
rm -rf spr*
split -a1 -b144 -x ../graphics/monsters1-charset.bin spr0
