GNULIB= ../gnulib/lib/libgnu.la
GNULIB_CFLAGS= -I $(top_builddir)/gnulib/lib -I $(top_srcdir)/gnulib/lib

AM_CFLAGS = @AUGEAS_CFLAGS@ @WARN_CFLAGS@ $(GNULIB_CFLAGS) $(LIBXML_CFLAGS)

AM_YFLAGS=-d -p spec_

EXTRA_DIST = try augeas_sym.version fa_sym.version

BUILT_SOURCES = datadir.h

DISTCLEANFILES = datadir.h

lib_LTLIBRARIES = libfa.la libaugeas.la
noinst_LTLIBRARIES = liblexer.la

bin_PROGRAMS = augtool augparse

include_HEADERS = augeas.h fa.h

libaugeas_la_SOURCES = augeas.h augeas.c augrun.c pathx.c \
	internal.h internal.c \
	memory.h memory.c ref.h ref.c \
    syntax.c syntax.h parser.y builtin.c lens.c lens.h regexp.c regexp.h \
	transform.h transform.c ast.c get.c put.c list.h \
    info.c info.h errcode.c errcode.h jmt.h jmt.c

if USE_VERSION_SCRIPT
  AUGEAS_VERSION_SCRIPT = $(VERSION_SCRIPT_FLAGS)$(srcdir)/augeas_sym.version
  FA_VERSION_SCRIPT = $(VERSION_SCRIPT_FLAGS)$(srcdir)/fa_sym.version
else
  AUGEAS_VERSION_SCRIPT =
  FA_VERSION_SCRIPT =
endif

libaugeas_la_LDFLAGS = $(AUGEAS_VERSION_SCRIPT) \
    -version-info $(LIBAUGEAS_VERSION_INFO)
libaugeas_la_LIBADD = liblexer.la libfa.la $(LIB_SELINUX) $(LIBXML_LIBS) $(GNULIB)

augtool_SOURCES = augtool.c
augtool_LDADD = libaugeas.la $(READLINE_LIBS) $(LIBXML_LIBS) $(GNULIB)

augparse_SOURCES = augparse.c
augparse_LDADD = libaugeas.la $(LIBXML_LIBS) $(GNULIB)

libfa_la_SOURCES = fa.c fa.h hash.c hash.h memory.c memory.h ref.h ref.c
libfa_la_LIBADD = $(LIB_SELINUX) $(GNULIB)
libfa_la_LDFLAGS = $(FA_VERSION_SCRIPT) -version-info $(LIBFA_VERSION_INFO)

liblexer_la_SOURCES = lexer.l
liblexer_la_CFLAGS = $(AM_CFLAGS) -Wno-error

FAILMALLOC_START ?= 1
FAILMALLOC_REP   ?= 20
FAILMALLOC_PROG ?= ./augtool

include $(top_srcdir)/Makefile.inc

# Generate datadir.h. AUGEAS_LENS_DIR in internal.h depends on
# the value of DATADIR
internal.h: datadir.h

FORCE-datadir.h: Makefile
	echo '#define DATADIR "$(datadir)"' > datadir.h1
	$(top_srcdir)/build/aux/move-if-change datadir.h1 datadir.h

datadir.h: FORCE-datadir.h
