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
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/select.h>
#include "uplog_common.h"
#include "uplog_util.h"
#include "uplogd.h"


#define  CONFIG_LINE_LENGTH  16384
#define  CONFIG_DATA_LENGTH  256

#define  BUFFER_LENGTH     MESSAGE_LENGTH + 4
#define  LOGMES_MAX_RETRY  3


#define  TRUE  1
#define  FALSE 0


struct struct_global_param{

	//char *program;
	int  daemon;
	
	char configfile[CONFIG_DATA_LENGTH];
	char logfile[CONFIG_DATA_LENGTH];
	char pidfile[CONFIG_DATA_LENGTH];
	char sockfile[CONFIG_DATA_LENGTH];

	int  socketfd;

}param;


struct struct_global_status{

	int socketfd; /* UNIX domain socket file descripter */

	int reload;   /* reload flag( reload=TRUE / FALSE) */ 
	int exit;     /* exit flag */

}status;


void exit_uplogd(int ret);
static void debug_print(void);



static void version(void)
{
	printf( "%s version %s\n", SERVER_PROGRAMNAME, VERSION );
	printf( "%s\n", AUTHOR );
}


static void usage(void)
{
	printf( "%s [-d] [-F] [-h] [-v] [-f CONFIGFILE]\n", SERVER_PROGRAMNAME );
	printf( "    -d: Debug mode(Print debug messages to stdout).\n" );
	printf( "    -F: Foreground mode.\n" );
	printf( "    -h: This help.\n" );
	printf( "    -v: Output version.\n" );
	printf( "    -f CONFIGFILE: Specify an configuration file.\n" );
}



void uplogd_handler(int signum)
{

	switch(signum){

	  case SIGHUP:
		status.reload = TRUE;
		break;

	  case SIGTERM:
		status.exit   = TRUE;
	}
	
}



/*
 * create_UNIX_socket:
 * 
 */
static int create_UNIX_socket(char *socketfile)
{
	int    sockfd;
	struct sockaddr_un addr;
	int    ret;

	/* initialize */
	memset( (char *)&addr, 0, sizeof(addr) );

	/* remove the socket file, if it exists. */
	(void) unlink( socketfile );

	/* create the UNIX domain socket. */
	sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if( sockfd < 0){
		err("Cannot create a UNIX domain socket: %s", strerror(errno));
		goto fail;
	}

	/* bind the socket */
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, socketfile);

	ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr) );
	if( ret < 0 ){
		err("Cannot bind(%s): %s", socketfile, strerror(errno));
		goto fail;
	}

	/* change the socket file mode(mode = rw-rw-rw-) */
	ret = chmod( socketfile, 0666 );
	if( ret < 0 ){
		err("Cannot chmod(file=%s): %s", socketfile, strerror(errno));
		goto fail;
	}

	return(sockfd);

fail:
	return(-1);
}


static void close_UNIX_socket(char *socketfile, int *sockfd)
{

	(void) close(*sockfd);

	(void) unlink( socketfile );

	*sockfd = -1;

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
		debug("LogMsg: %s", msg);
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

static int do_logging(void)
{

	fd_set fds, readfds;
	int    nfds;

	char   buf[BUFFER_LENGTH];
	int    ret;

	/* Create the UNIX socket */
	status.socketfd = create_UNIX_socket( param.sockfile );
	if( status.socketfd < 0 ){ goto fail; }



	FD_ZERO(&readfds);
	FD_SET(status.socketfd, &readfds);

	for(;;){

		memset(buf, 0, sizeof(buf));
		memcpy(&fds, &readfds, sizeof(fd_set));

		/* wait  */
		nfds = select( status.socketfd+1, (fd_set *)&readfds, (fd_set *)NULL,
		           (fd_set *)NULL, (struct timeval *)NULL);

		if( status.exit ){
			debug("exit loop");
			break;
		}

		if( nfds == 0 ){
			debug("no select activity.\n");
			continue;
		}

		if( nfds < 0 ){
			if( errno == EINTR ){
				info("interrupted\n");
				continue;
			}else{
				err("select error: %s\n", strerror(errno));
				goto fail;
			}
		}

		/* Recived message from Domain socket */
		if( FD_ISSET(status.socketfd, &readfds) ){
	
			ret = recv(status.socketfd, buf, BUFFER_LENGTH, 0 );
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


/*-------------------------------------------
 * Load the configuration file
 * 
 * ------------------------------------------
 */
enum {
	CONF_SEARCH_KEY,
	CONF_KEY,
	CONF_SEARCH_EQUAL,
	CONF_EQUAL,
	CONF_SEARCH_VALUE,
	CONF_VALUE,
	CONF_VALUE_QUOTATION,
	CONF_COMPLETE,
	CONF_NONE
};

static char *skipspace(char *line){

	char *pt;

	pt = line;
	while( isspace(*pt) ){
		pt++;
	}
	return(pt);
}

static int load_config(char *config){

	int  ret = FALSE;
	FILE *fp;
	char line[CONFIG_LINE_LENGTH];
	int  line_num = 0;

	debug("Load Config: (debug)Open %s",config);
	if( ( fp = fopen(config, "r") ) == NULL ){
		err("Load Config: Cannot open the configuration file. file=%s err=%s", config, strerror(errno));
		ret = FALSE;
		goto ReturnFunc;
	}

	debug("Load Config: (debug)Loading the configuration file.");
	debug("Load Config: (debug)--------------------------------------------------------");
	while( fgets(line, sizeof(line), fp)){

		struct container{
			char *pt;
			int  length;
		};
		struct container key   = { NULL, 0 };
		struct container value = { NULL, 0 };
		char   *p;
		char   mark;
		int    state;

		line_num++;

		/* Skip space & set status */
		p = skipspace(line);
		state = CONF_SEARCH_KEY;

		while( *p != '\n' && *p != '\0' ){

			/* Comment */
			if( *p == '#' && state != CONF_VALUE_QUOTATION ){
				break;
			}

			/* Perse strings */
			switch(state){

			  case CONF_SEARCH_KEY:
				if( ! isspace(*p) ){
					key.pt = p;
					key.length = 1;
					state = CONF_KEY;
				}
				break;

			  case CONF_KEY:
				if( isspace(*p) ){
					*p = '\0';
					state = CONF_SEARCH_EQUAL;
				}else if( *p == '=' ){
					*p = '\0';
					state = CONF_SEARCH_VALUE;
				}else{
					key.length++;
				}
				break;

			  case CONF_SEARCH_EQUAL:
				if( *p == '=' ){
					state = CONF_SEARCH_VALUE;
				}
				break;

			  case CONF_SEARCH_VALUE:
				if( ! isspace(*p) ){
					value.pt = p;
					value.length = 1;
					if( *p == '\"' || *p == '\'' ){
						mark  = *p;
						state = CONF_VALUE_QUOTATION;
					}else{
						state = CONF_VALUE;
					}
				}
				break;

			  case CONF_VALUE:
				if( isspace(*p)){
					*p    = '\0';
					state = CONF_COMPLETE;
				}else{
					value.length++;
				}
					
				break;

			  case CONF_VALUE_QUOTATION:
				if( *p == mark ){
					*p    = '\0';
					value.pt++;
					state = CONF_COMPLETE;
				}else{
					value.length++;
				}

				break;

			  default:
				err("load_config():BUG: illegal status.(status=%d)",stat);
				ret = FALSE;
				goto ReturnFunc;
			}
			if( state == CONF_COMPLETE ){
				break;
			}
			p++;
		}	

		/* Check result */
		if( state == CONF_SEARCH_KEY ){
			continue;
		}else if( state == CONF_VALUE ){
			*p = '\0';
		}else if( state != CONF_COMPLETE ){
			goto SyntaxError;

		}
		debug("Load Config: (debug)Line:%-3d key=%s value=%s",line_num, key.pt, value.pt);


		/* Search and set parameters */
		if( strncmp( key.pt, "logfile", key.length) == 0 ){
			strncpy( param.logfile, value.pt, CONFIG_DATA_LENGTH );
		
		}else if( strncmp( key.pt, "sockfile", key.length) == 0){
			strncpy( param.sockfile, value.pt, CONFIG_DATA_LENGTH );

		}else{
			err("Load Config: Unknown key(key=%s): line=%-3d file=%s", key.pt, line_num, config);
		}

	continue;

	SyntaxError:
		err(  "Load Config: Syntax error: line=%-3d file=%s", line_num, config);

	}
	debug("Load Config: (debug)--------------------------------------------------------");

	/* success */
	ret = TRUE;

ReturnFunc:
	if( fp != NULL ){	
		(void) fclose(fp);
	}
	return(ret);

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







void exit_uplogd(int ret)
{

	debug("close the socket and delete the socket file.");
	close_UNIX_socket( param.sockfile, &(status.socketfd));

	debug("delete the pid file.");

	exit(ret);
}


void debug_print(void)
{

	if( util_param.debug){
		debug("-----------------------------------------------");
		debug("<Parameter list>");
		debug(" Debug mode  :%d", util_param.debug);
		debug(" Deamon mode :%d", param.daemon);
		debug(" Config file :%s", param.configfile);
		debug(" Logfile     :%s", param.logfile);
		debug(" Pidfile     :%s", param.pidfile);
		debug(" Socketfile  :%s", param.sockfile);
		debug("-----------------------------------------------");
	}

}


int main(int argc, char **argv)
{
	int   ch;
	pid_t pid;
	int   ret;

	ret = EXIT_FAILURE;

	/* Set for printlog function */
	util_param.debug = FALSE;
	
	/* Set default values to global_parameter */
	memset( (char *)&param, 0, sizeof(param));
	param.daemon     = TRUE;
	strncpy( param.configfile, CONFFILE,    CONFIG_DATA_LENGTH-1);
	strncpy( param.logfile,    LOGFILE,     CONFIG_DATA_LENGTH-1);
	strncpy( param.pidfile,    PIDFILE,     CONFIG_DATA_LENGTH-1);
	strncpy( param.sockfile,   SOCKET_FILE, CONFIG_DATA_LENGTH-1);
	status.socketfd = -1;

	/* Set enviroments */
	(void)setlocale(LC_ALL, "C");


	/* Parse the command line. */
	while( ( ch = getopt(argc, argv, "dFf:hp:v") ) != EOF ){
		switch( (char)ch ){

		  case 'd': /* Debug mode      */
			util_param.debug = TRUE;
			break;

		  case 'F': /* Foreground mode */
		    param.daemon = FALSE;
			break;

		  case 'f': /* configuration file */
			memset( param.configfile, 0, CONFIG_DATA_LENGTH );
			strncpy(param.configfile, optarg,CONFIG_DATA_LENGTH-1);
			break;

		  case 'h': /* Help */
		    usage();
			ret = EXIT_SUCCESS;
			goto main_exit;
			break;  /* dummy */

		  case 'p': /* pid file */
			memset( param.pidfile, 0, CONFIG_DATA_LENGTH );
			strncpy(param.pidfile, optarg,CONFIG_DATA_LENGTH-1);
			break;

		  case 'v': /* Output version */
			version();
			ret = EXIT_SUCCESS;
			goto main_exit;
			break;  /* dummy */

		  default:
			usage();
			ret = EXIT_FAILURE;
			goto main_exit;
		}
	}

	/* Load the configuration file */
	if( ! load_config( param.configfile ) ){
		err("Cannot load the configuration file.");
		ret = EXIT_FAILURE;
		goto main_exit;
	}
	debug_print();


	/* Initalize process for daemon */
	if( param.daemon ){

		/* check */
		if( check_pid(param.pidfile) > 0 ){
			err("the pid file and pid alread exist");
			ret = EXIT_FAILURE;
			goto main_exit;
		}

		/* fork */
		debug("Fork a daemon process");
		pid = do_daemon( !util_param.debug );
		if( pid < 0){
			ret = EXIT_FAILURE;
			goto main_exit;
		}else if( pid != 0 ){
			/* parent process */
			ret = EXIT_SUCCESS;
			goto main_exit;
		}

		/* write the pid file */
		debug("Write the pid to the pid file(%s)",param.pidfile);
		if( write_pid(param.pidfile) < 0  ){
			err("Cannot write the pid file.");
			ret = EXIT_FAILURE;
			goto main_exit;
		}
	}


	/* main routine */
	ret = do_logging();

main_exit:
	debug("uplogd finalization & exit");
	exit_uplogd(ret);

}
