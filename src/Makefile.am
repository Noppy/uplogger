
AM_CFLAGS = \
    -DPREFIX_DIR='"$(prefix)"'            \
	-DSYSCONF_DIR='"$(sysconfdir)"'       \
    -DLOCALSTATE_DIR='"$(localstatedir)"' \
	-DLOG_DIR='"$(UPLOGGER_LOGDIR)"'      \
	-DPID_DIR='"$(UPLOGGER_PIDDIR)"'      \
	-DSOCK_DIR='"$(UPLOGGER_SOCKDIR)"'

sbin_PROGRAMS  = uplogd 
bin_PROGRAMS   = uplogger
uplogd_SOURCES = uplogd.c uplog_util.c \
                 uplog_common.h uplog_util.h uplogd.h config.h

uplogger_SOURCES = uplogger_cmd.c uplogger.c uplog_util.c \
                   uplog_common.h uplogger.h uplog_util.h config.h

lib_LIBRARIES = libuplogger.a
libuplogger_a_SOURCES = uplogger.c uplogger.h uplog_util.h uplog_common.h config.h

include_HEADERS = uplogger.h
