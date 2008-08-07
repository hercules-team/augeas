
GNULIB= ../gnulib/lib/libgnu.la
GNULIB_CFLAGS= -I $(top_srcdir)/gnulib/lib

AM_CFLAGS = @AUGEAS_CFLAGS@ @WARN_CFLAGS@ $(GNULIB_CFLAGS)

AM_YFLAGS=-d -p spec_

EXTRA_DIST=try augeas_sym.version

BUILT_SOURCES = datadir.h

DISTCLEANFILES = datadir.h

lib_LTLIBRARIES = libfa.la libaugeas.la
noinst_LTLIBRARIES = liblexer.la

bin_PROGRAMS = augtool augparse

include_HEADERS = augeas.h fa.h

libaugeas_la_SOURCES = augeas.h augeas.c \
	internal.h internal.c \
	memory.h memory.c ref.h \
        syntax.c syntax.h parser.y builtin.c lens.c lens.h regexp.c \
	transform.c get.c put.c list.h
libaugeas_la_LDFLAGS = -Wl,--version-script=$(srcdir)/augeas_sym.version \
        -version-info $(LIBAUGEAS_VERSION_INFO)
libaugeas_la_LIBADD = liblexer.la libfa.la $(GNULIB)

augtool_SOURCES = augtool.c
augtool_LDADD = libaugeas.la $(READLINE_LIBS) $(GNULIB)

augparse_SOURCES = augparse.c
augparse_LDADD = libaugeas.la $(GNULIB)

libfa_la_SOURCES = fa.c fa.h hash.c hash.h memory.c memory.h ref.h
libfa_la_LDFLAGS = -version-info $(LIBFA_VERSION_INFO)

liblexer_la_SOURCES = lexer.l
liblexer_la_CFLAGS = $(AM_CFLAGS) -Wno-error

# Generate datadir.h. AUGEAS_LENS_DIR in internal.h depends on
# the value of DATADIR
internal.h: datadir.h

FORCE-datadir.h: Makefile
	echo '#define DATADIR "$(abspath $(datadir))"' > datadir.h1
	$(top_srcdir)/build/aux/move-if-change datadir.h1 datadir.h

datadir.h: FORCE-datadir.h
