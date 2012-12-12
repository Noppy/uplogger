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


extern pid_t check_pid(char *pidfile_path);
extern pid_t write_pid(char *pidfile_path);
