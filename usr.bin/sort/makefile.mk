BINDIR    = /usr/local/bin
MANDIR    = /usr/man

# Nothing should need to be changed below this point

# DEFINES   = -DDEBUG -D__GNO__
DEFINES   = -D__GNO__
CFLAGS    = $(DEFINES) -O   -v -w -r
CFLAGS2   = $(DEFINES) -O31 -v -w -r
MAINFLAGS = $(DEFINES) -O   -v -w -S1024
LDFLAGS   = -v
# LDLIBS    = -l/usr/lib/gnulib -l/usr/lib/stack
LDLIBS    = -l/usr/lib/gnulib

MOBJS      = msort.o linecount.o loadarray.o
DOBJS      = dsort.o disksort.o initdisksort.o mergeone.o tempnam.o
COMMONOBJS = sortarray.o

install:
	/bin/cp msort dsort $(BINDIR)
	/bin/cp msort.1 dsort.1 $(MANDIR)/man1

all: msort dsort

msort	: $(MOBJS) $(COMMONOBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $<

dsort	: $(DOBJS) $(COMMONOBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $<

msort.o: msort.c common.h
	$(CC) -c $(MAINFLAGS) -o $@ msort.c

dsort.o: dsort.c common.h
	$(CC) -c $(MAINFLAGS) -o $@ dsort.c

#	Orca/C screws up with loop invariant optimization on disksort.c
disksort.o: disksort.c common.h
	$(CC) -c $(CFLAGS2) -o $@ disksort.c

#
# Housekeeping
#

clean:
	$(RM) $(DOBJS) $(MOBJS) $(COMMONOBJS) msort.root dsort.root

clobber:	clean
	$(RM) dsort msort

#
# Additional dependencies
#

linecount.o loadarray.o initdisksort.o mergeone.o sortarray.o:: common.h
