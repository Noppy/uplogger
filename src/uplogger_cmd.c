/* Uplogger
 * Copyright(C) 2012 N.Fujita All rights reserved.
 *
 * - logging command
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "uplogger.h"


#define BUFFER_LENGTH 4096

static char const space[] = " ";

int main(int argc, char **argv)
{

	char msg[BUFFER_LENGTH];
	int  ret;

	argc--;
	argv++;

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

	ret = uplogger(1, 1, "%s", msg);

	if( ret ){
		exit(EXIT_SUCCESS);
	}else{
		exit(EXIT_FAILURE);
	}
}

