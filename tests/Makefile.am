AUGTESTS=$(wildcard *.rb)

AM_CFLAGS = @AUGEAS_CFLAGS@ @WARN_CFLAGS@

valgrind: fatest
	libtool --mode=execute valgrind --quiet --leak-check=full ./fatest

EXTRA_DIST=augtest $(AUGTESTS) root \
	   test-lenses.sh grammars

noinst_SCRIPTS = test-lenses.sh

check_PROGRAMS = fatest

TESTS_ENVIRONMENT = top_builddir=$(abs_top_builddir) \
                    top_srcdir=$(abs_top_srcdir)

# We should also run augtest, but that doesn't work yet
TESTS = test-lenses.sh $(check_PROGRAMS)

INCLUDES = -I$(top_srcdir)/src

fatest_SOURCES = fatest.c cutest.c cutest.h
fatest_LDADD = @top_builddir@/src/libaugeas.la
