AC_INIT(augeas, 0.0.8)
AM_INIT_AUTOMAKE([-Wno-portability])

AC_GNU_SOURCE

AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_YACC
AC_PROG_LEX

AUGEAS_COMPILE_WARNINGS(error)

## Compiler flags to be used everywhere
AUGEAS_CFLAGS=-std=gnu99
AC_SUBST(AUGEAS_CFLAGS)

AUGEAS_CHECK_READLINE

AC_OUTPUT(Makefile
          src/Makefile
          man/Makefile
          tests/Makefile
          augeas.pc augeas.spec)
