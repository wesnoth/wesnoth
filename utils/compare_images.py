#!/usr/bin/env python
#
#  Script to compare images pixel-by-pixel to detect corruption due to
#  problems in tools such as optipng.
#
#  Takes two files as arguments, each being a list of image files.
#  Images are being compared between the two lists, one by one.

#  Run-time requirements: Python, PIL (Python Imaging Library)
#
#  Copyright (C) 2011 by Karol 'grzywacz' Nowak (grywacz@gmail.com)
#
#  Part of the Battle for Wesnoth Project <http://www.wesnoth.org>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2 or,
#  at your option any later version. This program is distributed in the
#  hope that it will be useful, but WITHOUT ANY WARRANTY. See the COPYING
#  file for more details.
#
from sys import argv

try:
    import Image as PIL
except ImportError, e:
    print "Unable to import PIL (Python Imaging Library)"
    raise e

list1 = open(argv[1])
list2 = open(argv[2])

for path1, path2 in zip(list1, list2):
    path1 = path1.strip()
    path2 = path2.strip()

    image1 = PIL.open(path1)
    image2 = PIL.open(path2)

    if image1.tostring() != image2.tostring():
        print path1 + " and " + path2 + " differ!"

# vim: ts=4:sw=4:expandtab
