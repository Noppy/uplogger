#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include "uplog_util.h"

void printlog(int priority, char *format, ...)
{

	va_list ap;
	char    msg[MES_LENGTH];
	FILE    *fp;

	va_start(ap, format);
	(void) vsnprintf( msg, MES_LENGTH, format, ap);
	va_end(ap);

	openlog( util_param.ident, LOG_PID, LOG_DAEMON);
	syslog( priority, msg);
	closelog();

	if( util_param.debug ){
		fprintf( stdout, "[%u] %s\n", getpid(), msg);

	}

}
