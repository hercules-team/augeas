SUBDIRS=src tests

AUGFILES=$(wildcard lenses/*.aug) $(wildcard lenses/tests/*.aug)

lensdir=$(datadir)/augeas/lenses

dist_lens_DATA=$(AUGFILES)

EXTRA_DIST=augeas.spec

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = augeas.pc

distclean-local:
	-rm -rf $(top_builddir)/build


