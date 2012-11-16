/*---------------------------------------------------------
 * uplogger - client library
 * 
 * - uplogger(char *sockfile,int add_header,int syslog,char *format, ...)
 *   Arguments
 *     - sockfile  : the path nape for a socket file.
 *                   NULL = default path(define SOCKET_FILE)
 *     - add_header: blooen, 1: Add header, 0:Do not add header
 *     - syslog    : blooen, 1: print error message to syslog
 *                           0: print error message to stderr
 *     - *format   : message
 *   Return value
 *     0 >= :On success
 *           (the unmber of characters which were written to uplog)
 *    -1    :On error
 *     
 *---------------------------------------------------------
 */

extern int uplogger( char *sockfile, int add_header, int syslog, char *format, ... );

