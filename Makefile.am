SUBDIRS=gnulib/lib src gnulib/tests tests man

ACLOCAL_AMFLAGS = -I gnulib/m4

lensdir=$(datadir)/augeas/lenses
lenstestdir=$(datadir)/augeas/lenses/tests

dist_lens_DATA=$(wildcard lenses/*.aug)
dist_lenstest_DATA=$(wildcard lenses/tests/*.aug)

EXTRA_DIST=augeas.spec

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = augeas.pc

distclean-local:
	-rm -rf $(top_builddir)/build

ChangeLog:
	@if test -d $(top_srcdir)/.hg; then \
		hg log --style changelog > $@; \
	 fi
dist: ChangeLog

.PHONY: ChangeLog
