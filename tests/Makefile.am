AUGTESTS=$(wildcard *.rb)

AM_CFLAGS = @AUGEAS_CFLAGS@ @WARN_CFLAGS@

valgrind: fatest
	libtool --mode=execute valgrind --quiet --leak-check=full ./fatest

EXTRA_DIST=augtest $(AUGTESTS) root \
	   test-grammars.sh grammars

noinst_SCRIPTS = test-grammars.sh

check_PROGRAMS = fatest

TESTS_ENVIRONMENT = top_builddir=$(abs_top_builddir) \
                    top_srcdir=$(abs_top_srcdir)

# We should also run test-grammars.sh, but the test grammars are busted
# right now
TESTS = augtest $(check_PROGRAMS)

INCLUDES = -I$(top_srcdir)/src

fatest_SOURCES = fatest.c cutest.c cutest.h
fatest_LDADD = @top_builddir@/src/libaugeas.la
