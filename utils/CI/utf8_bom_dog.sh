#!/bin/bash
set -e
if grep -qorHbm1 "^`echo -ne '\xef\xbb\xbf'`" po/ src/ data/ ; then
    echo "Found UTF8 BOM(s)!"
    grep -orHbm1 "^`echo -ne '\xef\xbb\xbf'`" po/ src/ data/
    exit 1
fi
