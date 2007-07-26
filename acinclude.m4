AC_DEFUN([AC_CHECK_EXTRA],[
  AC_ARG_WITH(
    extra-libs,
    [  --with-extra-libs=DIR   comma separated list of additional lib directories ],
    [
      EXTRA=`echo $withval | sed -e ':a;s/,/ -L/;t a'`
      LDFLAGS="$LDFLAGS -L$EXTRA"
    ]
  )
  AC_ARG_WITH(
    extra-includes,
    [  --with-extra-includes=DIR  comma separated list of additional include directories ],
    [
      EXTRA=`echo $withval | sed -e ':a;s/,/ -I/;t a'`
      CFLAGS="$CFLAGS -I$EXTRA"
    ]
  )
])
