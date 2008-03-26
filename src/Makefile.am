
AM_CFLAGS = @AUGEAS_CFLAGS@ @WARN_CFLAGS@ @GLIB_CFLAGS@

AM_YFLAGS=-d -p spec_

EXTRA_DIST=try augeas_sym.version

lib_LTLIBRARIES = libfa.la libaugeas.la
noinst_LTLIBRARIES = liblexer.la libnewlang.la

bin_PROGRAMS = augtool augparse syntax

include_HEADERS = augeas.h fa.h

libaugeas_la_SOURCES = augeas.h augeas.c config.h \
	internal.h internal.c \
	prov_spec.c spec-parse.y ast.h ast.c get.c put.c list.h
libaugeas_la_LDFLAGS = -Wl,--version-script=$(srcdir)/augeas_sym.version
libaugeas_la_LIBADD = liblexer.la libfa.la

augtool_SOURCES = augtool.c
augtool_LDADD = libaugeas.la $(READLINE_LIBS)

liblexer_la_SOURCES = spec-lex.l
liblexer_la_CFLAGS = $(AM_CFLAGS) -Wno-error

augparse_SOURCES = augparse.c
augparse_LDADD = libaugeas.la

libfa_la_SOURCES = fa.c fa.h
libfa_la_LDFLAGS = @GLIB_LIBS@

syntax_SOURCES = syntax.c syntax.h parser.y builtin.c lens.c lens.h
syntax_LDADD = libnewlang.la libaugeas.la

libnewlang_la_SOURCES = lexer.l
libnewlang_la_CFLAGS = $(AM_CFLAGS) -Wno-error
