AUGTESTS=$(wildcard *.rb)

AM_CFLAGS = @AUGEAS_CFLAGS@ @WARN_CFLAGS@

valgrind: fatest
	libtool --mode=execute valgrind --quiet --leak-check=full ./fatest

check_SCRIPTS=test-lenses.sh test-interpreter.sh

EXTRA_DIST=augtest $(AUGTESTS) root \
	   $(check_SCRIPTS) modules

noinst_SCRIPTS = $(check_SCRIPTS)

check_PROGRAMS = fatest

TESTS_ENVIRONMENT = top_builddir=$(abs_top_builddir) \
                    top_srcdir=$(abs_top_srcdir)

# We should also run augtest, but that doesn't work yet
TESTS = $(check_SCRIPTS) $(check_PROGRAMS)

INCLUDES = -I$(top_srcdir)/src

fatest_SOURCES = fatest.c cutest.c cutest.h
fatest_LDADD = @top_builddir@/src/libaugeas.la
