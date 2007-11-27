
AM_CFLAGS = @AUGEAS_CFLAGS@ @WARN_CFLAGS@

lib_LTLIBRARIES = libaugeas.la

bin_PROGRAMS = augtool

libaugeas_la_SOURCES = augeas.h augeas.c

augtool_SOURCES = augtool.c
augtool_LDADD = libaugeas.la $(AUGTOOL_LIBS)
