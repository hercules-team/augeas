SUBDIRS=src tests

AUGFILES=$(wildcard spec/*.aug)

data_DATA=$(AUGFILES)

EXTRA_DIST=doc/lenses.tex $(AUGFILES)

distclean-local:
	-rm -rf $(top_builddir)/build


