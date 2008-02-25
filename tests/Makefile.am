AUGTESTS=$(wildcard *.rb)

EXTRA_DIST=augtest $(AUGTESTS) root \
	   test-grammars.sh grammars

noinst_SCRIPTS = test-grammars.sh

TESTS_ENVIRONMENT = top_builddir=$(abs_top_builddir) \
                    top_srcdir=$(abs_top_srcdir)

# We should also run test-grammars.sh, but the test grammars are busted
# right now
TESTS = augtest
