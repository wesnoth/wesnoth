#!/bin/bash

make -C data/tools reindent
find src/ -name \*.\[ch\]pp -print0 | xargs -0 sed -i 's/[[:blank:]]*$//'
find data/lua/ -name \*.lua -print0 | xargs -0 sed -i 's/[[:blank:]]*$//'
