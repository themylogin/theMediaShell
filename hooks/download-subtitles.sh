#!/bin/bash
IFS="\n"
for i in `find -type f -regex ".*/.*\.\(avi\|mkv\|mp4\)"`;
do
    subliminal download -l en "$i"
done
