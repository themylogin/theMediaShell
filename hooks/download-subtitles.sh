#!/bin/bash
find -type f -regex ".*/.*\.\(avi\|mkv\|mp4\)" | sort | xargs subliminal download -l en "$i"
