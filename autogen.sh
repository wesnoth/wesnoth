#! /bin/sh

aclocal -I m4
autoheader
automake --add-missing --copy
autoconf
echo "*" > autom4te.cache/.cvsignore
