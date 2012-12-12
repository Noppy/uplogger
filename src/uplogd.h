/* Uplogd header file
 * -File path for uplogd
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#define SERVER_PROGRAMNAME "uplogd"


/*   */
#ifndef PREFIX_DIR
# define PREFIX_DIR "/usr/local/uplogger"
#endif

#ifndef SYSCONF_DIR
# define SYSCONF_DIR PREFIX_DIR"/etc"
#endif

#ifndef LOG_DIR
# define LOG_DIR PREFIX_DIR"/var/log"
#endif

#ifndef PID_DIR
# define PID_DIR PREFIX_DIR"/var/run"
#endif


/* File path */
#define LOGFILE       LOG_DIR"/uplog.log"
#define PIDFILE       PID_DIR"/"SERVER_PROGRAMNAME".pid"
#define CONFFILE      SYSCONF_DIR"/uplogd.conf"

