/* Uplogger
 * Copyright(C) 2012 N.Fujita All rights reserved.
 *
 * - Common utilities
 */

#include <syslog.h>

#define  MES_LENGTH    1024

#define debug(arg...) printlog(LOG_DEBUG, ## arg)
#define info(arg...)  printlog(LOG_INFO,  ## arg)
#define err(arg...)   printlog(LOG_ERR,   ## arg)


struct struct_util_param{
	char *ident;
	int  debug;
}util_param;

extern void printlog(int priority, char *format, ...);

