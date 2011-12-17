#!/bin/bash
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the NaCl-LICENSE file.

set -e

DST=$1

if [ z$DST == z ]; then
    echo "Need dstdir"
    exit 1
fi

if [ -d $DST ]; then
    echo "$DST already exists"
    exit 1
fi

ROOT=$NACL_TOOLCHAIN_ROOT/x86_64-nacl
WESNOTH=$ROOT/usr/local/share/wesnoth

mkdir $DST
cp -v $ROOT/wesnoth.html $DST/
cp -v $ROOT/wesnoth.nmf $DST/
cp -v $ROOT/wesnoth.js $DST/
cp -v $ROOT/check_browser.js $DST/
cp -v $ROOT/peppermount_helper.js $DST/

mkdir -p $DST/usr/local/bin/
cp -v $ROOT/usr/local/bin/wesnoth $ROOT/usr/local/bin/wesnoth32 $DST/usr/local/bin/

mkdir $DST/lib32 $DST/lib64
for lib in `cat $ROOT/wesnoth.nmf | grep '"url": "lib32' | perl -pe 's/^.*?url": "lib32\/(.*)".*/$1/'`; do
    cp -v $ROOT/lib32/$lib $DST/lib32/$lib
done
for lib in `cat $ROOT/wesnoth.nmf | grep '"url": "lib64' | perl -pe 's/^.*?url": "lib64\/(.*)".*/$1/'`; do
    cp -v $ROOT/lib64/$lib $DST/lib64/$lib
done

mkdir -p $DST/usr/local/share/wesnoth
cp -v $ROOT/usr/local/share/wesnoth/pack* $DST/usr/local/share/wesnoth/

mkdir -p $DST/usr/local/share/wesnoth/data/core/music/
cp -rv $ROOT/usr/local/share/wesnoth/data/core/music/* $DST/usr/local/share/wesnoth/data/core/music/

mkdir -p $DST/usr/local/share/wesnoth/fonts/
cp -rv $ROOT/usr/local/share/wesnoth/fonts/* $DST/usr/local/share/wesnoth/fonts/
