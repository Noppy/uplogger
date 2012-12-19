#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([uplogger], [1.0], [noppys2012@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/uplog_util.c])
AC_CONFIG_HEADERS([src/config.h])

# Set default prefix path 
AC_PREFIX_DEFAULT(/usr/local/uplog)

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


AC_ARG_WITH([logdir],
    [AS_HELP_STRING( [--with-logdir=DIR],
	                 [message log directory [DIR=LOCALSTATEDIR/log]] )],
	[UPLOGGER_LOGDIR="${withval}"],
	[UPLOGGER_LOGDIR="${localstatedir}/log"]
)

AC_ARG_WITH([piddir],
    [AS_HELP_STRING( [--with-piddir=DIR],
	                 [PID file directory [DIR=LOCALSTATEDIR/run]] )],
	[UPLOGGER_PIDDIR="${withval}"],
	[UPLOGGER_PIDDIR="${localstatedir}/run"]
)

AC_ARG_WITH([sockdir],
    [AS_HELP_STRING( [--with-sockdir=DIR],
	                 [socket file directory [DIR=LOCALSTATEDIR/run]] )],
	[UPLOGGER_SOCKDIR="${withval}"],
	[UPLOGGER_SOCKDIR="${localstatedir}/run"]
)


AC_SUBST(UPLOGGER_LOGDIR)
AC_SUBST(UPLOGGER_PIDDIR)
AC_SUBST(UPLOGGER_SOCKDIR)


AC_CONFIG_FILES([Makefile
                 src/Makefile
				 conf/Makefile
				 init-redhat/Makefile])


# Create library
AC_PROG_RANLIB

AC_OUTPUT

# For a trigger to run the creation of ./conf/uplogd.conf.
touch ./conf/FLAG
touch ./init-redhat/FLAG

