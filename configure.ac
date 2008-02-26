AC_INIT(augeas, 0.0.2)
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


AC_CHECK_LIB(readline, main, 
	[AUGTOOL_LIBS="$AUGTOOL_LIBS -lreadline"], 
	[AC_MSG_ERROR([readline library not found])],
	[$AUGTOOL_LIBS])
AC_SUBST(AUGTOOL_LIBS)

AC_OUTPUT(Makefile 
          src/Makefile
          tests/Makefile
          augeas.pc)
