#!/bin/bash

OS="$(uname -s)"

make -C data/tools reindent

if [[ "$OS" == "Linux" ]]; then
  find src/ -not -path "src/modules/*" -name \*.\[ch\]pp -print0 | xargs -0 sed -i -e '$a\'
  find src/ -not -path "src/modules/*" -name \*.\[ch\]pp -print0 | xargs -0 sed -i 's/[[:blank:]]*$//'
  find data/ -name \*.lua -print0 | xargs -0 sed -i 's/[[:blank:]]*$//'
else
  find src/ -not -path "src/modules/*" -name \*.\[ch\]pp -print0 | xargs -0 sed -i '' 's/[[:blank:]]*$//'
  find data/ -name \*.lua -print0 | xargs -0 sed -i '' 's/[[:blank:]]*$//'
fi

find data/ -name \*.lua -print0 | xargs -0 data/tools/check_mixed_indent
