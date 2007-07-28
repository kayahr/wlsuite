lib_LTLIBRARIES = libwasteland.la
libwasteland_la_LIBADD =
libwasteland_la_LDFLAGS = -release 0.1 -no-undefined
libwasteland_la_SOURCES = common.c vxor.c pic.c sprites.c
libwastelandincludedir = $(includedir)
libwastelandinclude_HEADERS = wasteland.h
AM_CFLAGS = -Wall -Werror -O2

doc:
	rm -f ../../doc/html/*.html ../../doc/html/*.gif
	mkdir -p ../../doc/html
	doc++ -M -H --dir ../../doc/html/ *.h *.c
