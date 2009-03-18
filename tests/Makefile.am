AUGTESTS=$(wildcard *.rb)

GNULIB= ../gnulib/lib/libgnu.la
GNULIB_CFLAGS= -I $(top_srcdir)/gnulib/lib

AM_CFLAGS = $(AUGEAS_CFLAGS) $(WARN_CFLAGS) $(GNULIB_CFLAGS) \
	-I $(top_builddir)/src

valgrind: fatest
	libtool --mode=execute valgrind --quiet --leak-check=full ./fatest

check_SCRIPTS=test-lenses.sh test-interpreter.sh test-get.sh \
              test-put-symlink.sh test-save-empty.sh test-mv.sh \
              test-bug-1.sh test-idempotent.sh test-preserve.sh \
	      test-events-saved.sh test-save-mode.sh test-unlink-error.sh

EXTRA_DIST=augtest $(AUGTESTS) root \
	   $(check_SCRIPTS) $(wildcard modules/*.aug) xpath.tests

noinst_SCRIPTS = $(check_SCRIPTS)

check_PROGRAMS = fatest test-xpath test-load

TESTS_ENVIRONMENT = \
  PATH='$(abs_top_builddir)/src$(PATH_SEPARATOR)'"$$PATH" \
  abs_top_builddir='$(abs_top_builddir)' \
  abs_top_srcdir='$(abs_top_srcdir)'

TESTS = $(check_SCRIPTS) $(check_PROGRAMS) augtest

INCLUDES = -I$(top_srcdir)/src

fatest_SOURCES = fatest.c cutest.c cutest.h
fatest_LDADD = $(top_builddir)/src/libaugeas.la $(GNULIB)

test_xpath_SOURCES = test-xpath.c
test_xpath_LDADD = $(top_builddir)/src/libaugeas.la $(GNULIB)

test_load_SOURCES = test-load.c cutest.c cutest.h
test_load_LDADD = $(top_builddir)/src/libaugeas.la $(GNULIB)
