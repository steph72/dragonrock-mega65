#!/bin/sh

# create reduced (128 characters) ingame charset 
# with leading dummy start address suitable to load via cbm kernel
# from the raw charset file

dd if=graphics/dr-charset.bin of=bin/drcharset_temp bs=1 count=2048
printf "00" | cat - bin/drcharset_temp > bin/drcharset
rm bin/drcharset_temp

