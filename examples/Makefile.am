
AM_CFLAGS = @AUGEAS_CFLAGS@ @WARN_CFLAGS@ @LIBXML_CFLAGS@ \
			-I $(top_srcdir)/src

bin_PROGRAMS = fadot
noinst_PROGRAMS = dump

fadot_SOURCES = fadot.c
fadot_LDADD = $(top_builddir)/src/libfa.la

dump_sources = dump.c
dump_LDADD =  $(top_builddir)/src/libaugeas.la $(top_builddir)/src/libfa.la
