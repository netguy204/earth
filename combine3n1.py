#!/usr/bin/env python

# combine a 3 channel image and a 1 channel image into a 4 channel
# image

from PIL import Image
import os
import sys

if __name__ == '__main__':
    three = Image.open(sys.argv[1])
    one = Image.open(sys.argv[2])
    out_name = sys.argv[3]

    tw, th = three.size
    ow, oh = one.size

    assert(tw == ow and th == oh)

    dest = Image.new("RGBA",  (tw, th))

    three_data = three.load()
    one_data = one.load()
    dest_data = dest.load()

    for ii in range(three.size[0]):
        for jj in range(three.size[1]):
            p1 = three_data[ii, jj]
            p2 = one_data[ii, jj]
            dest_data[ii, jj] = (p1[0], p1[1], p1[2], p2[0])

    dest.save(out_name)
