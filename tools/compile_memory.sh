#!/bin/bash

make clean

for f in test/src/*.cpp
do
    rss=`/usr/bin/time -v make debug/$f.o 2>&1 | grep "Maximum resident set size" | rev | cut -d" " -f1 | rev`
    memory=$(echo "scale=2; $rss/1024" | bc -l)
    echo "$f => $rss => ${memory}MB"
done