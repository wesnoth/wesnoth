#!/bin/bash
# strip_image_profiles.sh
# Copyright (C) 2006 by Nils Kneuper <crazy-ivanovic AT gmx DOT net>
# Part of the Battle for Wesnoth Project http://www.wesnoth.org
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY.
#
# See the COPYING file for more details.
#
#
# Scipt to strip ICC profiles from all png files within Wesnoth
#
# Requirements: bash, imagemagick, optipng
# HowToUse: start the script from the wesnoth maindir
# make sure that the correct files are about to be commited
# enter a commit message
# enjoy ICC profile clean png images

find . -name '*.png' -exec grep -li profil {} \; > images_to_convert
for i in `cat images_to_convert`; do convert -strip $i $i;  optipng -q -o5 -nb -nc -np $i; done
svn ci
rm images_to_convert

# end strip_image_profiles.sh
