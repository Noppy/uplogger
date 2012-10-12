/* Uplogger
 * Copyright(C) 2012 N.Fujita All rights reserved.
 *
 * - Server daemon
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/select.h>
#include "uplog_common.h"
#include "uplog_util.h"
#include "uplogd.h"


#define  PROGRAMNAME       "uplogd"
#define  VERSION           "0.1"
#define  AUTHOR            "Written by N.Fujita."


#define  DATA_LENGTH       256

#define  BUFFER_LENGTH     MESSAGE_LENGTH + 4
#define  LOGMES_MAX_RETRY  3


#define  TRUE  1
#define  FALSE 0


struct struct_global_param{

	char *program;
	int  daemon;

	char logfile[DATA_LENGTH];
	char pidfile[DATA_LENGTH];

}param;



static void version()
{
	printf( "%s version %s\n", PROGRAMNAME, VERSION );
	printf( "%s\n", AUTHOR );
}


static void usage()
{
	printf( "%s [-dFh]\n", PROGRAMNAME );
	printf( "    -d: Print debug messages to stdout\n" );
	printf( "    -F: Foreground mode\n" );
	printf( "    -h: This help\n" );
}


/*
 * create_UNIX_socket:
 * 
 */
static int create_UNIX_socket()
{
	int    sockfd;
	struct sockaddr_un addr;
	int    ret;

	/* initialize */
	memset( (char *)&addr, 0, sizeof(addr) );

	/* remove the socket file, if it exists. */
	(void) unlink(SOCKET_FILE);

	/* create the UNIX domain socket. */
	sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if( sockfd < 0){
		err("Cannot create a UNIX domain socket: %s", strerror(errno));
		goto fail;
	}

	/* bind the socket */
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SOCKET_FILE);

	ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr) );
	if( ret < 0 ){
		err("Cannot bind(%s): %s",SOCKET_FILE, strerror(errno));
		goto fail;
	}

	/* change the socket file mode(mode = rw-rw-rw-) */
	ret = chmod( SOCKET_FILE, 0666 );
	if( ret < 0 ){
		err("Cannot chmod(file=%s): %s", SOCKET_FILE, strerror(errno));
		goto fail;
	}

	return(sockfd);

fail:
	return(-1);
}


static void logmessage(char *msg)
{
	int log_fd;
	int ret, size;
	int count, flag;

	flag = FALSE;
	for( count=0; count < 3; count++){ 
		/* open the log file */
		log_fd = open(param.logfile, O_WRONLY|O_CREAT|O_APPEND|O_NOCTTY,
		                             S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
		if( log_fd < 0 ){
			err("Cannot open the log file(%s): %s", param.logfile, strerror(errno));
			continue;
		}
		/* calculate length of the message */
		size = strnlen( msg, BUFFER_LENGTH );
		/* write message */
		ret = write( log_fd, msg, size );
		if( ret < 0 ){
			/* occurred write error */
			err("Cannot write to the log file: %s", strerror(errno));
			(void)close( log_fd );
			continue;
		}else{
			if( ret != size ){
				err("Writing was interrupted to the log file");
				flag = FALSE;
			}else{
				/* write complete */
				flag = TRUE;
			}
			break;
		}
	}
	if( ! flag ){
		err("Write to the log file was aborted.(MAX=%d)", LOGMES_MAX_RETRY);
		info("Save the MES:%s",msg);
	}

	/* close the log file */
	if( log_fd > 0 ){
		(void)close( log_fd );
	}
}


static void printline(char *msg)
{

	char buf[BUFFER_LENGTH];
	register char *msg_pt, *buf_pt;
	register unsigned char c;

	/* Initialize */
	msg_pt = msg;

	/* Convert Characters */
	memset( buf, 0, sizeof(buf) );
	buf_pt = buf;

	while( (c = *msg_pt++) && buf_pt < &buf[sizeof(buf)-4] ){
		if( c == '\0' ){
			/* NULL */
			break;
		}else if( c <= 0x1F || c == 0x7F ){
			/*  < 0x1F : Control characters,  0x7F : DEL */
			*buf_pt++ = ' ';
		}else{
			/* other characters */
			*buf_pt++ = c;
		}
	}
	*buf_pt++ = '\n';
	*buf_pt   = '\0';

	/* log a message to the log file */
	logmessage( buf );


}

static int do_logging()
{

	int    socketfd;

	fd_set fds, readfds;
	int    nfds;

	char   buf[BUFFER_LENGTH];
	int    ret;

	/* Create the UNIX socket */
	socketfd = create_UNIX_socket();
	if( socketfd < 0 ){ goto fail; }



	FD_ZERO(&readfds);
	FD_SET(socketfd, &readfds);

	for(;;){

		memset(buf, 0, sizeof(buf));
		memcpy(&fds, &readfds, sizeof(fd_set));

		nfds = select( socketfd+1, (fd_set *)&readfds, (fd_set *)NULL,
		           (fd_set *)NULL, (struct timeval *)NULL);

		if( nfds == 0 ){
			debug("no select activity.\n");
			continue;
		}

		if( nfds < 0 ){
			if( errno == EINTR ){
				info("interrupted\n");

/* under const */

				continue;
			}else{
				err("select error: %s\n", strerror(errno));
				goto fail;
			}
		}

		/* Recived message from Domain socket */
		if( FD_ISSET(socketfd, &readfds) ){
	
			ret = recv(socketfd, buf, BUFFER_LENGTH, 0 );
			if( ret < 0 ){
				if( errno == EINTR ){ continue; };

				err("Socket read error: %s\n", strerror(errno));
				goto fail;
			}

			/* Print the message */
			printline(buf);
			
		}
	}


	return(EXIT_SUCCESS);

fail:
	return(EXIT_FAILURE);

}

/* check_pid:
 * 
 *  pid does not exist: ret = -1
 *  pid exists        : ret = pid
 */
pid_t check_pid()
{

	FILE *file;
	pid_t  pid;

	/* open the pid file for read */
	if( (file = fopen( param.pidfile, "r")) == NULL ){
		debug("Cannot open the pidfile(%s): %s", param.pidfile, strerror(errno));
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
 *    success: ret = pid
 *    failure: ret = -1
 */
pid_t write_pid()
{
	int   fd;
	FILE  *file;
	pid_t pid;

	/* open the pid file. */
	fd = open( param.pidfile, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH );
	if( fd == -1 ){
		err("Cannot open or create pidfile(%s): %s", param.pidfile, strerror(errno));
		goto fail;
	}
	if( (file = fdopen(fd, "w") ) == NULL ){;
		err("Cannot open or create pidfile(%s): %s", param.pidfile, strerror(errno));
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
	(void)unlink(param.pidfile);
	return(-1);

}




static pid_t do_daemon(int close_interface)
{

	pid_t ret;
	int   fd;

	/*create child process and terminate parent process*/
	ret = fork();
	if( ret < 0 ){
		/* fork error */
		err("Cannot fork: %s", strerror(errno));
		return(-1);
	}else if( ret != 0 ){
		/* parent process */
		return(ret);
	
	}
	/* child process */

	/* change chiled process to session gruop reader */
	ret = setsid();
	if( ret < 0 ){
		err("Cannot create a session: %s", strerror(errno));
		return(-1);
	}

	/* create 2nd child process and terminate parent process again
	 * to never connect terminal
	 */
	ret = fork();
	if( ret < 0 ){
		/* fork error */
		err("Cannot fork: %s", strerror(errno));
		return(-1);
	}else if( ret != 0 ){
		/* parent process */
		return(ret);
	}

	/* change the current directory to root directory */
	(void) chdir("/");

	/* close stdin/stdout/stderr */
	fd = open("/dev/null", O_RDWR);
	if( fd < 0 ){
		err("Cannot open NULL device: %s", strerror(errno));
		return(-1);
	}

	/* close the STDIN file descriptor */
	if ((fd != STDIN_FILENO) && (dup2(fd, STDIN_FILENO) < 0)){
		err("Cannot close the STDIN: %s", strerror(errno));
		(void)close(fd);
		return(-1);
	}

	/* close　the STDOUT and STDERR file descriptor, if option is TRUE */
	if( close_interface ){
		/* close the STDOUT file descriptor */
		if ((fd != STDOUT_FILENO) && (dup2(fd, STDOUT_FILENO) < 0)) {
			err("Cannot close the STDOUT: %s", strerror(errno));
			(void)close(fd);
			return(-1);
		}

		/* close the STDERR file descriptor */
		if ((fd != STDERR_FILENO) && (dup2(fd, STDERR_FILENO) < 0)) {
			err("Cannot close the STDERR: %s", strerror(errno));
			(void)close(fd);
			return(-1);
		}
	}

	if( fd > STDERR_FILENO ){
		(void)close(fd);
	}
	
	/* daemon complete */
	return(0);

}



int main(int argc, char **argv)
{
	int   ch;
	pid_t pid;
	int   ret;

	ret = EXIT_FAILURE;

	/* set default parameter */
	util_param.debug = FALSE;

	param.daemon     = TRUE;
	strncpy( param.logfile, LOGFILE, DATA_LENGTH);
	strncpy( param.pidfile, PIDFILE, DATA_LENGTH);

	
	/* set enviroments */
	(void)setlocale(LC_ALL, "C");


	/* Parse the command line. */
	while( ( ch = getopt(argc, argv, "dFhv") ) != EOF ){
		switch( (char)ch ){

		  case 'd': /* Debug mode      */
			util_param.debug = TRUE;
			break;

		  case 'F': /* Foreground mode */
		    param.daemon = FALSE;
			break;

		  case 'h': /* Help */
		    usage();
			ret = EXIT_SUCCESS;
			goto exit;
			break;  /* dummy */

		  case 'v': /* Display version */
			version();
			ret = EXIT_SUCCESS;
			goto exit;
			break;  /* dummy */

		  default:
			usage();
			ret = EXIT_FAILURE;
			goto exit;
		}
	}


	/* initalize process for daemon */
	if( param.daemon ){

		/* check */
		if( check_pid(param.pidfile) > 0 ){
			err("the pid file and pid alread exist");
			ret = EXIT_FAILURE;
			goto exit;
		}

		/* fork */
		debug("Fork a daemon process");
		pid = do_daemon( !util_param.debug );
		if( pid < 0){
			ret = EXIT_FAILURE;
			goto exit;
		}else if( pid != 0 ){
			/* parent process */
			ret = EXIT_SUCCESS;
			goto exit;
		}

		/* write the pid file */
		debug("Write the pid to the pid file(%s)",param.pidfile);
		if( write_pid(param.pidfile) < 0  ){
			err("Cannot write the pid file.");
			ret = EXIT_FAILURE;
			goto exit;
		}
	}

	


	/* main routine */
	ret = do_logging();

exit:
	
	exit(ret);

}
