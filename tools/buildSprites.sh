#!/bin/sh

#create single sprite files (144 bytes) out of monstersX-charset.bin

echo "building sprites..."

cd bin

rm -rf spr*
split -a1 -b144 ../graphics/monsters0-charset.bin spra
split -a1 -b144 ../graphics/monsters1-charset.bin sprb
rm spr?o
cd ..
