#!/bin/sh

make -C data/tools reindent

# use -i for GNU sed and -i '' otherwise
sed --version 2>/dev/null | grep -q 'GNU sed' || i=1

find src -path src/modules -prune -o \
	\( -name '*.[cht]pp' -o -name '*.[ch]' -o -name '*.mm' \) -type f \
	-exec sed -i ${i:+''} -e 's/[[:blank:]]*$//' -e '$a\' {} +

find data -name '*.lua' -type f \
	-exec sed -i ${i:+''} -e 's/[[:blank:]]*$//' -e '$a\' {} + \
	-exec data/tools/check_mixed_indent {} +
