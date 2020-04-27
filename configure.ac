AC_PREREQ([2.69])
AC_INIT([cpsrksec],[0.1],[gkobeaga@bcamath.org])

AC_CONFIG_SRCDIR([src/cpsrksec.h])

AC_CONFIG_MACRO_DIR([m4])

dnl AM_INIT_AUTOMAKE
AM_INIT_AUTOMAKE([subdir-objects])
AM_PROG_AR

AC_CONFIG_HEADERS([config.h])

dnl Check for programs
AC_PROG_CC
AC_PROG_INSTALL
LT_INIT

dnl Checks for libraries.
AC_CONFIG_MACRO_DIR([m4])

LT_INIT

# Checks for header files.
AC_CHECK_HEADERS([ stdlib.h math.h string.h strings.h errno.h assert.h stddef.h unistd.h malloc.h sys/types.h ])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_CONFIG_FILES([Makefile src/Makefile exp/Makefile])
AC_OUTPUT
