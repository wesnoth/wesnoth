#!/bin/bash
set -e
if grep -qorHbm1 "^`echo -ne '\xef\xbb\xbf'`" po/ src/ data/ ; then
    echo "Error, Found a UTF8 BOM:\n"
    grep -orHbm1 "^`echo -ne '\xef\xbb\xbf'`" po/ src/ data/
    exit 1
fi
