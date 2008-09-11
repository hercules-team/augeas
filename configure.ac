AC_INIT(augeas, 0.3.1)
AC_CONFIG_SRCDIR([src/augeas.c])
AC_CONFIG_AUX_DIR([build/aux])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([-Wno-portability])


dnl Check for NaturalDocs
AC_ARG_WITH([naturaldocs],
  [AS_HELP_STRING([--with-naturaldocs=DIR],
    [path to NaturalDocs install dir to generate documentation])], 
  [AC_CHECK_FILE($withval/NaturalDocs,
      [AC_PATH_PROG(PERL, perl, no)
      if test "x$PERL" = "xno"; then
           AC_MSG_ERROR(You need a working installation of Perl to use NaturalDocs)
      fi
      NATURALDOCSDIR=$withval
      HAS_NATURALDOCS=naturaldocs
      ],
      [ AC_MSG_ERROR(You asked to use NaturalDocs but it could not be found)])
  ])
AC_SUBST(HAS_NATURALDOCS)
AC_SUBST(NATURALDOCSDIR)
AC_SUBST(PERL)

dnl NaturalDocs output format, defaults to HTML
NATURALDOCSFORMAT=HTML
AC_ARG_WITH([naturaldocs-output],
  [AS_HELP_STRING([--with-naturaldocs-output=FORMAT],
    [format of NaturalDocs output (possible values: HTML/FramedHTML, default: HTML)])],
  [
    case $withval in
       HTML|FramedHTML)
          NATURALDOCSFORMAT=$withval
	  ;;
       *)
          AC_MSG_ERROR($withval is not a supported output format for NaturalDocs)
	  ;;
    esac
  ])
AC_SUBST(NATURALDOCSFORMAT)


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

dnl Version info in libtool's notation
AC_SUBST([LIBAUGEAS_VERSION_INFO], [1:0:1])
AC_SUBST([LIBFA_VERSION_INFO], [0:0:0])

AC_GNU_SOURCE

AC_PROG_CC
gl_EARLY


dnl gl_INIT uses m4_foreach_w, yet that is not defined in autoconf-2.59.
dnl In order to accommodate developers with such old tools, here's a
dnl replacement definition.
m4_ifndef([m4_foreach_w],
  [m4_define([m4_foreach_w],
    [m4_foreach([$1], m4_split(m4_normalize([$2]), [ ]), [$3])])])

AC_PROG_LIBTOOL
AC_PROG_YACC
AC_PROG_LEX

AUGEAS_COMPILE_WARNINGS(error)

## Compiler flags to be used everywhere
AUGEAS_CFLAGS=-std=gnu99
AC_SUBST(AUGEAS_CFLAGS)

AUGEAS_CHECK_READLINE
AC_CHECK_FUNCS([open_memstream])

gl_INIT

AC_OUTPUT(Makefile \
          gnulib/lib/Makefile \
          gnulib/tests/Makefile \
          src/Makefile \
          man/Makefile \
          tests/Makefile \
	  doc/Makefile \
	  doc/naturaldocs/Makefile \
          augeas.pc augeas.spec)
