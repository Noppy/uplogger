/* Uplogger
 * - Client library modules
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
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "uplog_common.h"
#include "uplog_util.h"
#include "uplogger.h"

#define  HEADER_LENGTH    256

#define  FALSE  0
#define  TRUE   1


/**********************************************************************
 * func: construct header strings 
 *
 * args: *head    : pointer of message string
 *       size     : the length of head array
 * ret : 
 *       none
 * ********************************************************************
 */
static void getheader( char *head, int size )
{

	time_t timer;
	struct tm *t_st;
	char   host[128];

	/* get time */
	(void)time( &timer );
	t_st = localtime( &timer );

	/* get hostname */
	(void)gethostname( host, sizeof(host) );

	/* construct header strings */
	snprintf( head, size, "%04d/%02d/%02d %02d:%02d:%02d %s ", 
	                t_st->tm_year + 1900,
					t_st->tm_mon  + 1,
	                t_st->tm_mday,
					t_st->tm_hour,
					t_st->tm_min,
					t_st->tm_sec,
					host );

}

/**********************************************************************
 * func: uplogger client(the core function)
 *
 * args: sockfile = NULL: default path(SOCKET_FILE) other: used specifed path
 *       add_header = 1: Log header message, 0:don't header message
 *       syslog     = 1: Log err to syslog,  0:Log err to stderr
 *       char *format ... = logging message
 * ret : 
 *       0 >= successful
 *      -1  = failure 
 * ********************************************************************
 */
int uplogger( char *sockfile, int add_header, int syslog, char *format, ... )
{

	va_list ap;
	char    *msg, *pt;
	char    head[HEADER_LENGTH  ];
	int     size;

	int     sockfd;
	struct  sockaddr_un addr;
	int     ret;

	/* initialize */
	ret = -1;

	/* malloc a array for themessage strings */
	if( ( msg = (char *)malloc( sizeof(char) * MESSAGE_LENGTH ) ) == NULL ){;
		if( syslog ){
			err(   "uplogger(): Cannot malloc: %s", strerror(errno));
		}else{
			perror("uplogger(): Cannot malloc");
		}
		goto func_ret;
	}
	pt = msg;

	/* add header string(yyyy/mm/dd hh:mm:ss host), if add_header is true */
	if( add_header ){
		getheader(msg, HEADER_LENGTH );
		size = strnlen( msg, MESSAGE_LENGTH);
		if( size < MESSAGE_LENGTH ){
			pt += size;
		}else{
			if( syslog ){
				err("uplogger(): the length of header is too long(msg=%d header=%d)",
				     MESSAGE_LENGTH, size);
			}else{
				fprintf(stderr, "uplogger(): the length of header is too long(msg=%d header=%d)\n", 
			         MESSAGE_LENGTH, size);
			}
			goto func_ret;
		}
	}

	/* construct message */
	va_start(ap, format);
	(void) vsnprintf( pt, MESSAGE_LENGTH - size, format, ap);
	va_end(ap);

	/* set the length of message */
	size = strnlen( msg, MESSAGE_LENGTH);


	/* initialize sockaddr */
	memset( (char *)&addr, 0, sizeof(addr) );

	/* create UNIX domain socket */
	sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if( sockfd < 0){
		if( syslog ){
			err(   "uplogger(): Cannot create a UNIX domain socket: %s", strerror(errno));
		}else{
			perror("uplogger(): Cannot create a UNIX domain socket");
		}
		goto func_ret;
	}

	/* construct the socket address */
	addr.sun_family = AF_UNIX;
	if( sockfile == NULL ){
		strcpy(addr.sun_path, SOCKET_FILE);
	}else{
		strcpy(addr.sun_path, sockfile);
	}
	/* send to message */
	ret = sendto(sockfd, msg, size, 0, 
	             (struct sockaddr *)&addr, sizeof(addr) );
	if( ret < 0 ){
		if( syslog ){
			err(   "uplogger(): Cannot send message: %s", strerror(errno));
		}else{
			perror("uplogger(): Cannot send message");
		}
	}

func_ret:

	(void) close(sockfd);
	free(msg);

	return(ret);
}
