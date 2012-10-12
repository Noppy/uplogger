

CC = gcc
LDFLAGS= -s



all: uplogd uplogger

uplogd: uplogd.o uplog_util.o
	${CC} ${LDFLAGS} -o uplogd uplogd.o uplog_util.o

uplogger: uplogger_cmd.o uplogger.o uplog_util.o
	${CC} ${LDFLAGS} -o uplogger uplogger_cmd.o uplogger.o uplog_util.o

uplogd.o: uplogd.c uplog_common.h uplogd.h
	${CC} -c uplogd.c

uplogger_cmd.o: uplogger_cmd.c uplogger.h
	${CC} -c uplogger_cmd.c

uplogger.o: uplogger.c uplogger.h uplog_common.h
	${CC} -c uplogger.c

uplog_util.o: uplog_util.c
	${CC} -c uplog_util.c

clean:
	rm -f uplogd uplogger *.o
