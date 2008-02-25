
AM_CFLAGS = @AUGEAS_CFLAGS@ @WARN_CFLAGS@

AM_YFLAGS=-d -p spec_

lib_LTLIBRARIES = libaugeas.la
noinst_LTLIBRARIES = liblexer.la

bin_PROGRAMS = augtool augparse

libaugeas_la_SOURCES = augeas.h augeas.c config.h \
	internal.h internal.c \
	prov_spec.c spec-parse.y ast.h ast.c parser.c put.c list.h

libaugeas_la_LIBADD = liblexer.la

augtool_SOURCES = augtool.c
augtool_LDADD = libaugeas.la $(AUGTOOL_LIBS)

liblexer_la_SOURCES = spec-lex.l
liblexer_la_CFLAGS = $(AM_CFLAGS) -Wno-unused-parameter -Wno-unused-function

augparse_SOURCES = augparse.c
augparse_LDADD = libaugeas.la
