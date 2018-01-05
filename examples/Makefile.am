
GNULIB= ../gnulib/lib/libgnu.la
GNULIB_CFLAGS= -I $(top_srcdir)/gnulib/lib

AM_CFLAGS = @AUGEAS_CFLAGS@ @WARN_CFLAGS@ @LIBXML_CFLAGS@ $(GNULIB_CFLAGS) \
			-I $(top_srcdir)/src

bin_PROGRAMS = fadot
noinst_PROGRAMS = dump

fadot_SOURCES = fadot.c
fadot_LDADD = $(top_builddir)/src/libfa.la $(GNULIB)

dump_sources = dump.c
dump_LDADD =  $(top_builddir)/src/libaugeas.la $(top_builddir)/src/libfa.la \
	          $(GNULIB)
