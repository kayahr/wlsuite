AC_PREREQ([2.50])
AC_INIT([wlsuite], [0.1], [k@ailis.de])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AC_LANG_C
AM_PROG_LIBTOOL

AC_CHECK_HEADERS(gd.h,,echo "ERROR: gd.h not found"; exit 1;)
AC_CHECK_LIB(gd,gdImageCreate,,echo "ERROR: GD library not found"; exit 1;)
AC_CHECK_HEADERS(kaytils/str.h,,echo "ERROR: kaytils/str.h not found"; exit 1;)
AC_CHECK_HEADERS(kaytils/list.h,,echo "ERROR: kaytils/list.h not found"; exit 1;)
AC_CHECK_LIB(kaytils,strCreate,,echo "ERROR: kaytils library not found"; exit 1;)

AC_DEFINE(AUTHOR,"Klaus Reimer",Authors name)
AC_DEFINE(EMAIL,"k@ailis.de",Authors email address)
AC_DEFINE(COPYRIGHT,"Copyright (C) 2010  Klaus Reimer <k@ailis.de>",Copyright message)

AC_CONFIG_FILES(
  Makefile 
  src/Makefile
  src/libwasteland/Makefile
  src/decodehuffman/Makefile
  src/encodehuffman/Makefile
  src/decodepic/Makefile
  src/encodepic/Makefile
  src/unpacksprites/Makefile
  src/packsprites/Makefile
  src/unpackcursors/Makefile
  src/packcursors/Makefile
  src/unpackfont/Makefile
  src/packfont/Makefile
  src/unpackcpa/Makefile
  src/packcpa/Makefile
  src/decodecpa/Makefile
  src/unpacktiles/Makefile
  src/unpackpics/Makefile
)
AC_OUTPUT
