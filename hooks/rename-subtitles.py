#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import pprint

video = sorted(sum([filter(lambda s: s.lower().endswith("." + ext), os.listdir("."))
                    for ext in ("avi", "mkv", "mp4")], []),
               key=lambda s: s.lower())
subs  = sorted(sum([filter(lambda s: s.lower().endswith("." + ext), os.listdir("."))
                    for ext in ("ass", "srt")], []),
               key=lambda s: s.lower())

if len(video) != len(subs):
    print "len(video) = %d, len(subs) = %d" % (len(video), len(subs))
else:
    rename = [(sub, ".".join(video.split(".")[:-1]) + "." + sub)
              for video, sub in zip(video, subs)]
    pprint.PrettyPrinter().pprint(rename)
    if raw_input("Rename (y/n)? ") == "y":
        for a, b in rename:
            os.rename(a, b)
