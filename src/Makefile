CC     = gcc
CFLAGS = 
LDFLAGS= -s



all: uplogd uplogger

uplogd: uplogd.o uplog_util.o
	${CC} ${CFLAGS} ${LDFLAGS} -o uplogd uplogd.o uplog_util.o

uplogger: uplogger_cmd.o uplogger.o uplog_util.o
	${CC} ${CFLAGS} ${LDFLAGS} -o uplogger uplogger_cmd.o uplogger.o uplog_util.o

uplogd.o: uplogd.c uplog_common.h uplogd.h
	${CC} ${CFLAGS} -c uplogd.c

uplogger_cmd.o: uplogger_cmd.c uplogger.h
	${CC} ${CFLAGS} -c uplogger_cmd.c

uplogger.o: uplogger.c uplogger.h uplog_common.h
	${CC} ${CFLAGS} -c uplogger.c

uplog_util.o: uplog_util.c
	${CC} ${CFLAGS} -c uplog_util.c

clean:
	rm -f uplogd uplogger *.o
