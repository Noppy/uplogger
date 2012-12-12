/* uplogger - client library
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
 *
 *
 *---------------------------------------------------------
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

