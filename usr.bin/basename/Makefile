#	Makefile for basename(1) and dirname(1)
#
#	Use TESTFLAGS to check stack usage.

CFLAGS    += -w -O -s 768 -c
LDFLAGS	  += -O
TESTFLAGS += -w -G25 -DSTACK_CHECK

all 			: basename dirname

basename		: basename.o basename.r
	$(CC) $(LDFLAGS) -o $@ $@.o
	$(CATREZ) -d $@ $@.r

dirname			: dirname.o dirname.r
	$(CC) $(LDFLAGS) -o $@ $@.o
	$(CATREZ) -d $@ $@.r


