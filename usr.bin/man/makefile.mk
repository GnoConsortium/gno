#
# Makefile for dmake(1)
#

# Location for executable.  It should be /usr/sbin

BINDIR = /usr/sbin

# Location for man pages.

MANDIR = /usr/man

#
# Nothing past this point should have to be changed
#
# -DISGRAPH_FIX should come out when Orca/C's isgraph() function is fixed.
#
# -DTEST_PROCESS is for debugging process().  Use "dmake process".
# -DDEBUG will give stack usage.
#

CFLAGS = -w -O -v -DISGRAPH_FIX -s1024

OBJS = makewhatis.o fillbuffer.o process.o
LDLIBS = -l/usr/lib/gnulib -l/usr/lib/stack
         
makewhatis: $(OBJS)
	$(CC) $(OBJS) $(LDLIBS) -o makewhatis

makewhatis.o: makewhatis.c makewhatis.h
	$(CC) -c $(CFLAGS) makewhatis.c

fillbuffer.o: fillbuffer.c makewhatis.h
	$(CC) -c $(CFLAGS) fillbuffer.c

process.o: process.c makewhatis.h
	$(CC) -c $(CFLAGS) process.c

install:
	/bin/cp makewhatis   $(BINDIR)
	/bin/cp makewhatis.1 $(MANDIR)/man1

clean:
	/bin/cp -p rm $(OBJS)

#
# These are just for debugging purposes
#

fillbuffer: fillbuffer.c makewhatis.h
	$(CC) $(CFLAGS) -DTEST_FILLBUFFER -o fillbuffer fillbuffer.c

process: process.o fillbuffer.o
	$(CC) $(CFLAGS) process.o fillbuffer.o -o process
