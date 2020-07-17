#!/bin/bash

set -e

cfile=BUILDIDX

if [ -f "$cfile" ]; then
    echo "incrementing $cfile"
    current=`cat $cfile`
    current=$[current+1]
    echo $current>$cfile
else 
    echo "creating $cfile"
    echo "1">$cfile
fi

