SUBDIRS=src tests

AUGFILES=$(wildcard spec/*.aug)

lensdir=$(DESTDIR)$(datadir)/augeas/lenses

dist_lens_DATA=$(AUGFILES)

EXTRA_DIST=doc/lenses.tex

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = augeas.pc

distclean-local:
	-rm -rf $(top_builddir)/build


