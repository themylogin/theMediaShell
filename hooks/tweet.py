#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import absolute_import, division, unicode_literals

import itertools
import os
from PIL import Image
import shutil
import subprocess
import sys
from tempfile import NamedTemporaryFile
import twitter

def main(directory, consumer_key, consumer_secret, access_token_key, access_token_secret):
    subprocess.call(["gwenview", directory])
    
    images = [x for x in os.listdir(directory) if x.endswith(b".jpg")]
    if not images:
        return

    collages = os.path.join(directory, b"collages")
    if not os.path.exists(collages):
        os.mkdir(collages)

    for i in itertools.count():
        collage_dir = os.path.join(collages, b"collage%04d" % i)
        if not os.path.exists(collage_dir):
            os.mkdir(collage_dir)
            break

    collage_path = os.path.join(collage_dir, b"collage.jpg")
    if len(images) == 1:
        shutil.copy(os.path.join(directory, images[0]), collage_path)
    else:
        bigw, bigh = Image.open(os.path.join(directory, images[0])).size
        w, h = map(lambda x: int(x / 2), (bigw, bigh))

        hborder = int(w * 0.05)
        vborder = int(h * 0.05)

        w -= int(hborder / 2)
        h -= int(vborder / 2)

        cw = (w + hborder) * 2 + hborder
        ch = (h + vborder) * (int(len(images) / 2)) + vborder
        if len(images) % 2:
            ch += bigh + vborder

        im = Image.new("RGB", (cw, ch), "black")
        for i, image in enumerate(images):
            im2 = Image.open(os.path.join(directory, image))
            if len(images) % 2 and i == len(images) - 1:
                im.paste(im2, (hborder, vborder + int(len(images) / 2) * (h + vborder)))
            else:
                im2.thumbnail((w, h), Image.ANTIALIAS)
                im.paste(im2, (hborder + (w + hborder) * (i % 2), vborder + int(i / 2) * (h + vborder)))

        im.save(collage_path)

    subprocess.call(["gwenview", collage_path])

    f = NamedTemporaryFile()
    f.write(os.path.basename(directory))
    f.flush()

    while True:
        try:
            tweet = subprocess.check_output(["zenity",
                                             "--title", "Tweet",
                                             "--text-info", "--editable",
                                             "--filename", f.name,
                                             "--width", "1280",
                                             "--height", "320",
                                             "--font", "Segoe UI 35"])
        except:
            return

        decoded = ""
        try:
            decoded = tweet.decode("utf-8")
            twitter.Api(consumer_key=consumer_key,
                        consumer_secret=consumer_secret,
                        access_token_key=access_token_key,
                        access_token_secret=access_token_secret).PostMedia(decoded, collage_path)
        except Exception as e:
            subprocess.check_call(["zenity", "--error", "--no-markup", "--text", repr(e) + b" (len=%d)" % len(decoded)])
            f.truncate(0)
            f.seek(0)
            f.write(tweet)
            f.flush()
        else:
            break

    for image in images:
        shutil.move(os.path.join(directory, image), collage_dir)


if __name__ == "__main__":
    main(*sys.argv[1:])
