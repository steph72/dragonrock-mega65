#!/bin/sh

#create single sprite files (144 bytes) out of monstersX-charset.bin

cd bin
rm -rf spr*
split -a2 -b144 -d ../graphics/monsters1-charset.bin spr0
