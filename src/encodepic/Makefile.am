bin_PROGRAMS = encodepic
encodepic_LDADD = ../libwasteland/libwasteland.la -lgd
encodepic_SOURCES = \
	encodepic.c

man_MANS = encodepic.1
EXTRA_DIST = $(man_MANS)

AM_CFLAGS = -Wall -Werror -O2 -I../libwasteland

encodepic.1:
	help2man -N ./encodepic > encodepic.1
