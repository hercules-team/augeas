
GNULIB= ../gnulib/lib/libgnu.la
GNULIB_CFLAGS= -I $(top_srcdir)/gnulib/lib

AM_CFLAGS = @AUGEAS_CFLAGS@ @WARN_CFLAGS@ $(GNULIB_CFLAGS) \
			-I $(top_builddir)/src

bin_PROGRAMS = fadot

fadot_SOURCES = fadot.c 
fadot_LDADD = $(top_builddir)/src/libaugeas.la $(GNULIB)


