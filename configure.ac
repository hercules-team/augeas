AC_INIT(augeas, 1.0.0)
AC_CONFIG_SRCDIR([src/augeas.c])
AC_CONFIG_AUX_DIR([build/aux])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([-Wno-portability 1.11 color-tests parallel-tests])
AM_SILENT_RULES([yes]) # make --enable-silent-rules the default.


dnl Check for NaturalDocs
AC_PATH_PROGS([ND_PROG], [naturaldocs NaturalDocs], missing)
AM_CONDITIONAL([ND_ENABLED], [test "x$ND_PROG" != "xmissing"])

dnl NaturalDocs output format, defaults to HTML
ND_FORMAT=HTML
AC_ARG_WITH([naturaldocs-output],
  [AS_HELP_STRING([--with-naturaldocs-output=FORMAT],
    [format of NaturalDocs output (possible values: HTML/FramedHTML, default: HTML)])],
  [
    if test "x$ND_PROG" = "xmissing"; then
      AC_MSG_ERROR([NaturalDocs was not found on your path; there's no point in setting the output format])
    fi
    case $withval in
       HTML|FramedHTML)
          ND_FORMAT=$withval
	  ;;
       *)
          AC_MSG_ERROR($withval is not a supported output format for NaturalDocs)
	  ;;
    esac
  ])
AC_SUBST(ND_FORMAT)


dnl Check for pdflatex
PDFDOCS=""
AC_ARG_WITH([pdfdocs],
  [AS_HELP_STRING([--with-pdfdocs],
    [whether to use pdflatex to build PDF docs])],
  [AC_PATH_PROG(PDFLATEX, pdflatex, no)
   if test "x$PDFLATEX" = "xno"; then
   	AC_MSG_ERROR(You asked to use PDFLatex but it could not be found)
   else
   	PDFDOCS="pdfdocs"
   fi
  ])
AC_SUBST(PDFLATEX)
AC_SUBST(PDFDOCS)

dnl Support for memory tests with failmalloc
AC_ARG_WITH([failmalloc],
  [AS_HELP_STRING([--with-failmalloc=FAILMALLOC],
     [enable failmalloc test targets and use the failmalloc library FAILMALLOC])],
  [AC_SUBST([LIBFAILMALLOC], ["$with_failmalloc"])],
  [with_failmalloc=no])

AM_CONDITIONAL([WITH_FAILMALLOC], [test x$with_failmalloc != xno])

dnl --enable-debug=(yes|no)
AC_ARG_ENABLE([debug],
              [AC_HELP_STRING([--enable-debug=no/yes],
                             [enable debugging output])],[],[enable_debug=yes])
AM_CONDITIONAL([ENABLE_DEBUG], test x"$enable_debug" = x"yes")
if test x"$enable_debug" = x"yes"; then
   AC_DEFINE([ENABLE_DEBUG], [1], [whether debugging is enabled])
fi

dnl Version info in libtool's notation
AC_SUBST([LIBAUGEAS_VERSION_INFO], [16:0:16])
AC_SUBST([LIBFA_VERSION_INFO], [5:0:4])

AC_GNU_SOURCE

AC_PROG_CC
gl_EARLY
AC_SYS_LARGEFILE

dnl gl_INIT uses m4_foreach_w, yet that is not defined in autoconf-2.59.
dnl In order to accommodate developers with such old tools, here's a
dnl replacement definition.
m4_ifndef([m4_foreach_w],
  [m4_define([m4_foreach_w],
    [m4_foreach([$1], m4_split(m4_normalize([$2]), [ ]), [$3])])])

AC_PROG_LIBTOOL
AC_PROG_YACC
AC_PROG_LEX

AUGEAS_COMPILE_WARNINGS(maximum)

## Compiler flags to be used everywhere
AUGEAS_CFLAGS=-std=gnu99
AC_SUBST(AUGEAS_CFLAGS)

AUGEAS_CHECK_READLINE
AC_CHECK_FUNCS([open_memstream uselocale])

VERSION_SCRIPT_FLAGS=
$(/usr/bin/ld --help 2>&1 | grep -- --version-script >/dev/null) && \
    VERSION_SCRIPT_FLAGS=-Wl,--version-script=
test "`uname`" == "SunOS" && \
    VERSION_SCRIPT_FLAGS="-Wl,-M -Wl,"
AC_SUBST(VERSION_SCRIPT_FLAGS)
AM_CONDITIONAL([USE_VERSION_SCRIPT], [test -n "$VERSION_SCRIPT_FLAGS"])

gl_INIT

PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([LIBXML], [libxml-2.0])

AC_CHECK_FUNCS([strerror_r fsync])

AC_OUTPUT(Makefile \
          gnulib/lib/Makefile \
          gnulib/tests/Makefile \
          src/Makefile \
          man/Makefile \
          tests/Makefile \
          examples/Makefile \
	  doc/Makefile \
	  doc/naturaldocs/Makefile \
          augeas.pc augeas.spec)
