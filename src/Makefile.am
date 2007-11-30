
AM_CFLAGS = @AUGEAS_CFLAGS@ @WARN_CFLAGS@ @LIBPCRE_CFLAGS@

lib_LTLIBRARIES = libaugeas.la

bin_PROGRAMS = augtool

libaugeas_la_SOURCES = augeas.h augeas.c \
	internal.h internal.c \
	provider.h \
	prov_pam.h prov_pam.c \
	record.h record.c

libaugeas_la_LIBADD = @LIBPCRE_LIBS@

augtool_SOURCES = augtool.c
augtool_LDADD = libaugeas.la $(AUGTOOL_LIBS)
