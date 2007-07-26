bin_PROGRAMS = decodepic
decodepic_LDADD = ../libwasteland/libwasteland.la -lgd
decodepic_SOURCES = \
	decodepic.c

man_MANS = decodepic.1
EXTRA_DIST = $(man_MANS)

AM_CFLAGS = -Wall -Werror -O2 -I../libwasteland

decodepic.1:
	help2man -N ./decodepic > decodepic.1
