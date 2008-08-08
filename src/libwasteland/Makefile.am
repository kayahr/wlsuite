lib_LTLIBRARIES = libwasteland.la
libwasteland_la_LIBADD =
libwasteland_la_LDFLAGS = -version-info 0:0:0 -no-undefined
libwasteland_la_SOURCES = \
  common.c \
  image.c \
  images.c \
  vxor.c \
  io.c \
  huffman.c \
  pic.c \
  sprites.c \
  cursors.c \
  font.c \
  cpa.c \
  msq.c \
  tiles.c
libwastelandincludedir = $(includedir)
libwastelandinclude_HEADERS = wasteland.h
AM_CFLAGS = -Wall -Werror -O2

doc:
	rm -f ../../doc/html/*.html ../../doc/html/*.gif
	mkdir -p ../../doc/html
	doc++ -M -H --dir ../../doc/html/ *.h *.c
