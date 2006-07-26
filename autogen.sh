#! /bin/sh

rm -rf autom4te.cache
aclocal -I m4
autoheader
automake --add-missing --copy
autoconf
