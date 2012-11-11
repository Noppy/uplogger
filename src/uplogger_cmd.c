/* Uplogger
 * Copyright(C) 2012 N.Fujita All rights reserved.
 *
 * - logging command
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "uplogger.h"

#define PROGRAMNAME "uplogger"
#define VERSION "0.1"
#define AUTHOR "Written by N.Fujita."

#define BUFFER_LENGTH 4096

static char const space[] = " ";


void version()
{
	printf( "%s version %s\n", PROGRAMNAME, VERSION );
	printf( "\n");
	printf( "%s\n", AUTHOR );
}

void usage()
{
	printf( "%s [-h] [-s SOCKETFILE] [-v] message\n", PROGRAMNAME);
	printf( "    -h: This help.\n");
	printf( "    -s: Specify UNIX Domain socket file.\n");
	printf( "    -v: Show version.\n");
	printf("\n");

}



int main(int argc, char **argv)
{

	char msg[BUFFER_LENGTH];
	char sock[BUFFER_LENGTH];
	int  ret;
	int  ch;

	/* Initialize */
	memset( sock, '\0',      BUFFER_LENGTH   );

	/* Perse arguments */
	while( ( ch = getopt(argc, argv, "hs:v") ) != EOF ){

		switch( (char)ch ){

		  case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;

		  case 's':
			strncpy(sock, optarg, BUFFER_LENGTH-1 );
			break;

		  case 'v':
			version();
			exit(EXIT_SUCCESS);
			break;

		  default:
			usage();
			exit(EXIT_FAILURE);
			break;
			
		}
	}
	argc -= optind;
	argv += optind;

	/* concatenate the arguments(separated by blank) */
	memset(msg, 0, BUFFER_LENGTH);
	while( argc > 0 ){
		(void)strncat(msg, *argv, BUFFER_LENGTH);
		argc--;
		argv++;
		
		/* add a blank, if it's not the last argument */
		if( argc > 0 ){
			(void)strncat(msg, space, BUFFER_LENGTH);
		}	
	}

	if( *sock == '\0' ){
		ret = uplogger(NULL, 1, 1, "%s", msg);
	}else{
		ret = uplogger(sock, 1, 1, "%s", msg);
	}

	if( ret ){
		exit(EXIT_SUCCESS);
	}else{
		exit(EXIT_FAILURE);
	}
}

