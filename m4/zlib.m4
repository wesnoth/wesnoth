dnl
dnl zlib.m4 -- Find the path to the zlib library.
dnl
AC_DEFUN([AM_PATH_ZLIB],
[ZLIB_CFLAGS=""
ZLIB_LDFLAGS=""
ZLIB_LIBS=""

AC_ARG_WITH([zlib-prefix], 
            [  --with-zlib-prefix=PFX  Prefix where zlib is installed (optional)],
            zlib_prefix="$withval", zlib_prefix="")
if test x"$zlib_prefix" = xyes ; then
  zlib_prefix="";
fi

AC_MSG_CHECKING([for zlib location])
if test x"$zlib_prefix" = x ; then
  zlib_header_found="no"
  for dir in /usr/local /usr ; do
  if test -f "$dir/include/zlib.h" ; then
    zlib_header_found="yes"
    ZLIB_CFLAGS="-I$dir/include"
    ZLIB_LDFLAGS="-L$dir/lib"
    break
  fi
  done
  if test x"$zlib_header_found" = "no" ; then
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([zlib header file 'zlib.h' not found.])
  fi
  AC_MSG_RESULT([$dir])
else
  ZLIB_CFLAGS="-I$zlib_prefix/include"
  ZLIB_LDFLAGS="-L$zlib_prefix/lib"
  AC_MSG_RESULT([$zlib_prefix])
fi

zlib_save_LDFLAGS=$LDFLAGS
LDFLAGS="$ZLIB_LDFLAGS $LDFLAGS"
zlib_save_CFLAGS=$CFLAGS
CFLAGS="$ZLIB_CFLAGS $CFLAGS"
AC_CHECK_LIB([z], [compress], [ZLIB_LIBS="$ZLIB_LDFLAGS -lz"], [ZLIB_LIBS=""])
LDFLAGS=$zlib_save_LDFLAGS
CFLAGS=$zlib_save_CFLAGS
if test x"$ZLIB_LIBS" = x ; then
  AC_MSG_ERROR([zlib required but not found])
fi

AC_SUBST([ZLIB_CFLAGS])
AC_SUBST([ZLIB_LIBS])])
]
