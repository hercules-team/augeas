AC_INIT(augeas, 0.2.2)
AC_CONFIG_SRCDIR([src/augeas.c])
AC_CONFIG_AUX_DIR([build/aux])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([-Wno-portability])

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
          augeas.pc augeas.spec)
