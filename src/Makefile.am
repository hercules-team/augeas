
AM_CFLAGS = @AUGEAS_CFLAGS@ @WARN_CFLAGS@

AM_YFLAGS=-d -p spec_

EXTRA_DIST=try

lib_LTLIBRARIES = libaugeas.la libfa.la
noinst_LTLIBRARIES = liblexer.la

bin_PROGRAMS = augtool augparse

include_HEADERS = augeas.h fa.h

libaugeas_la_SOURCES = augeas.h augeas.c config.h \
	internal.h internal.c \
	prov_spec.c spec-parse.y ast.h ast.c parser.c put.c list.h

libaugeas_la_LIBADD = liblexer.la libfa.la

augtool_SOURCES = augtool.c
augtool_LDADD = libaugeas.la $(READLINE_LIBS)

liblexer_la_SOURCES = spec-lex.l
liblexer_la_CFLAGS = $(AM_CFLAGS) -Wno-error

augparse_SOURCES = augparse.c
augparse_LDADD = libaugeas.la

libfa_la_SOURCES = fa.c fa.h
