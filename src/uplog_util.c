/* Uplogger
 * - Common utilities
 *
 * Copyright 2012 N.Fujita <noppys2012@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "uplog_util.h"


/* --------------------------------------------------------------
 * message logging to syslog
 * 
 * (void) printlog(prio, char *format, ...)
 *	 prio: Conform to the syslog function
 *	       * LOG_ERR,LOG_WARNING,LOG_INFO...
 *	         (refer "man 3 syslog")
 *	 *format: message
 * --------------------------------------------------------------
 */
void printlog(int priority, char *format, ...)
{

	va_list   ap;
	char	  msg[MES_LENGTH];
	FILE	  *fp;
	time_t	timer;
	struct tm *t;

	va_start(ap, format);
	(void) vsnprintf( msg, MES_LENGTH, format, ap);
	va_end(ap);

	openlog( util_param.ident, LOG_PID, LOG_DAEMON);
	syslog( priority, msg);
	closelog();

	/* Print message to stdout, if debug mode. */
	if( util_param.debug ){
		/* set time */
		(void)time( &timer );
		t = localtime( &timer );

		fprintf( stdout, "%04d/%02d/%02d %02d:%02d:%02d uplogd[%u]: %s\n",
			t->tm_year + 1900,
			t->tm_mon  + 1,
			t->tm_mday,
			t->tm_hour,
			t->tm_min,
			t->tm_sec,
			getpid(), msg);

	}else if( priority <= LOG_ERR ){
		fprintf( stdout, "uplogd: %s\n", msg);
	}
}


/* --------------------------------------------------------------
 * Interact with pid files
 * 
 * (pid_t) check_pid(char *pidfile_path)
 *     ret = pid: uplogd is already exists.
 *     ret = -1 : uplogd isn't exists, or cannot read pidfile.
 *
 * (pid_t) write_pid(char *pidfile_path)
 *     ret = pid: success for writing pidfile.
 *     ret = -1 : failure
 * --------------------------------------------------------------
 */
pid_t check_pid(char *pidfile_path)
{

	FILE *file;
	pid_t  pid;

	/* open the pid file for read */
	if( (file = fopen( pidfile_path, "r")) == NULL ){
		debug("Cannot open the pidfile(%s): %s", pidfile_path, strerror(errno));
		goto fail;
	}

	/* read pid and close the file */
	(void)fscanf(file, "%d", &pid);
	(void)fclose(file);

	/* check pid */
	if( pid <= 0 || pid == getpid() ){
		err("pid is invalid or myself(pid=%d)", pid);
		goto fail;
	}

	/* check the process */
	if( kill( pid, 0 ) && errno == ESRCH ){
		/* not found process */
		goto fail;
	}

	/* pid exists */
	return(pid);

fail:
	return(-1);
}


/* write_pid:
 *
 * Writes the pid to the specified file.
 *	success: ret = pid
 *	failure: ret = -1
 */
pid_t write_pid(char *pidfile_path)
{
	int   fd;
	FILE  *file;
	pid_t pid;

	/* open the pid file. */
	fd = open( pidfile_path, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH );
	if( fd == -1 ){
		err("Cannot open or create pidfile(%s): %s", pidfile_path, strerror(errno));
		goto fail;
	}
	if( (file = fdopen(fd, "w") ) == NULL ){;
		err("Cannot open or create pidfile(%s): %s", pidfile_path, strerror(errno));
		goto fail;
	}

	/* get pid */
	pid = getpid();

	/* write pid */
	if( fprintf(file, "%d\n", pid) == 0 ){
		err("Cannot write pid: %s", strerror(errno));
		goto fail;
	}
	(void)fflush(file);

	/* close the pid file。*/
	(void)fclose(file);

	return(pid);

fail:
	(void)close(fd);
	(void)unlink(pidfile_path);
	return(-1);

}
