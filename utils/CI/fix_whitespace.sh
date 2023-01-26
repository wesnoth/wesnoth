#!/bin/bash

OS="$(uname -s)"

make -C data/tools reindent

if [[ "$OS" == "Linux" ]]; then
  find src/ -name \*.\[ch\]pp -print0 | xargs -0 sed -i 's/[[:blank:]]*$//'
  find data/ -name \*.lua -print0 | xargs -0 sed -i 's/[[:blank:]]*$//'
  find data/ -name \*.lua -print0 | xargs -0 sed -i 's/\t/    /g'
else
  find src/ -name \*.\[ch\]pp -print0 | xargs -0 sed -i '' 's/[[:blank:]]*$//'
  find data/ -name \*.lua -print0 | xargs -0 sed -i '' 's/[[:blank:]]*$//'
  find data/ -name \*.lua -print0 | xargs -0 sed -i '' 's/\t/    /g'
fi
