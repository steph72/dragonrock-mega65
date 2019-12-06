#!/bin/sh

#create single sprite files (144 bytes) out of monstersX-charset.bin

echo "building sprites..."

cd bin
rm -rf spr*
split -a1 -b144 ../graphics/monsters0-charset.bin spra
split -a1 -b144 ../graphics/monsters1-charset.bin sprb

cd ..

c1541 disc/drock.d64 -delete spr*

for filename in bin/spr*; do
  c1541 disc/drock.d64 -write $filename
done

