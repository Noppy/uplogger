#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([uplogger], [1.0], [noppys2012@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/uplog_util.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h locale.h stdlib.h string.h sys/socket.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_STRNLEN
AC_CHECK_FUNCS([dup2 getcwd gethostname memset realpath select setlocale socket strerror])

AC_CONFIG_FILES([Makefile
                 src/Makefile])


AC_PROG_RANLIB

AC_OUTPUT
