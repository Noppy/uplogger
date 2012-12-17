#!/bin/bash
#
# uplogd        Startup script for rsyslog.
#
# chkconfig: 2345 13 89
# description: Message logging system for User Program(shell, etc. ).
### BEGIN INIT INFO
# Provides: $uplogd
# Required-Start: $local_fs
# Required-Stop: $local_fs
# Default-Start:  2 3 4 5
# Default-Stop: 0 1 6
# Short-Description: uplog daemon
# Description: Message logging system for User Program(shell, etc. ). 
### END INIT INFO

# Source function library.
. /etc/init.d/functions

RETVAL=0
PIDFILE=/var/run/uplogd.pid

prog=uplogd
exec=/usr/local/uplog/sbin/uplogd
lockfile=/var/lock/subsys/$prog

# Source config
if [ -f /etc/sysconfig/$prog ] ; then
    . /etc/sysconfig/$prog
fi

function start() {
	[ -x $exec ] || exit 5

	umask 077

        echo -n $"Starting uplogger: "
        daemon --pidfile="$PIDFILE" $exec -p "$PIDFILE" $UPLOGD_OPTIONS
        RETVAL=$?
        echo
        [ $RETVAL -eq 0 ] && touch $lockfile
        return $RETVAL
}

function stop() {
        echo -n $"Shutting down uplogger: "
        killproc -p "$PIDFILE" $exec
        RETVAL=$?
        echo
        [ $RETVAL -eq 0 ] && rm -f $lockfile
        return $RETVAL
}

function rhstatus() {
        status -p "$PIDFILE" -l $prog $exec
}

function restart() {
        stop
        start
}

function reload() {
        echo -n $"Reloading uplogger:"
        killproc -p "$PIDFILE" $exec -HUP
		RETVAL=$?
		echo
		return $RETVAL
}


case "$1" in
  start)
        start
        ;;
  stop)
        stop
        ;;
  restart)
        restart
        ;;
  reload)
        reload
        ;;
  force-reload)
        restart
        ;;
  status)
        rhstatus
        ;;
  *)
        echo $"Usage: $0 {start|stop|restart|reload|force-reload|status}"
        exit 3
esac

exit $?
