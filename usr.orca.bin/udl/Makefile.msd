#
# Makefile for udl
# (c) 1993-1996 Soenke Behrens
#

# DJGPP is a gcc port
CC	= gcc

# Where do we put the binaries and man page?

BINDIR	= /djgpp/bin
MANDIR	= /djgpp/man

# OS-dependant macros.  See the README for an explanation of these.
DEFINES	= -DREADDIR_RETURNS_DOT -DHAS_ATEXIT

INSTALL	= cp -f
CFLAGS  = $(DEFINES) -O2

#
# You should not have to modify anything beyond this point
#

OBJS	= udl.o udluse.o common.o globals.o

udl: $(OBJS)
	$(CC) $(LDFLAGS) -o udl $(OBJS) $(LDLIBS)

udl.o: udlunix.c common.h
	$(CC) -c $(CFLAGS) -o udl.o udlunix.c

install:
	$(INSTALL) udl $(DESTDIR)$(BINDIR)
	$(INSTALL) udl.1 $(DESTDIR)$(MANDIR)

clean:
	-rm *.o *~ core

clobber: clean
	-rm udl

udluse.o common.o globals.o:: common.h

