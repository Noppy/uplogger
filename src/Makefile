CC     = gcc
CFLAGS = -O2
LDFLAGS= -s
DEFS   =
LIBS   =

PROG_UPLOGD   = uplogd
PROG_UPLOGGER = uplogger


all: $(PROG_UPLOGD) $(PROG_UPLOGGER)

$(PROG_UPLOGD): uplogd.o uplog_util.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

uplogger: uplogger_cmd.o uplogger.o uplog_util.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

.c.o:
	$(CC) -c $(CFLAGS) $<

uplogd.o:       uplogd.h   uplog_util.h uplog_common.h
uplogger_cmd.o: uplogger.h
uplogger.o:     uplogger.h uplog_util.h uplog_common.h
uplog_util.o:   uplog_util.h


clean:
	rm -f uplogd uplogger *.o
