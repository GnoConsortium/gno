#
# Makefile for udl
# Copyright (c) 1993-1996 Soenke Behrens, Devin G. Reade
#
# This makefile should be used with dmake.
#
# $Id: Makefile.gs,v 1.9 1996/02/04 01:34:23 gdr Exp $
#

# Where do we put the binaries and man page?

BINDIR	= /usr/local/bin
MANDIR	= /usr/local/man

# OS-dependant macros.  See the README for an explanation of these.

DEFINES = -DGNO -D_POSIX_C_SOURCE -D_POSIX_SOURCE -DHAS_ATEXIT \
	  -DOVERFLOW_CHECK

# Use optimization and a 2k stack.

CFLAGS	= $(DEFINES) -O -w -s2048
LDFLAGS	= -s2048

# Depending on how you have your libraries set up, you may not need
# this next line.  In that case, just comment it out.

LDLIBS	= -l/usr/lib/gnulib

#
# You should not have to modify anything beyond this point
#

OBJS	= udl.o udluse.o common.o globals.o

udl: $(OBJS) udl.r help/udl
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	copyfork udl.r $@ -r

udl.o: udlgs.c common.h
	$(CC) -c $(CFLAGS) -o udl.o udlgs.c

install:
	cp -f udl   $(BINDIR)
	cp -f udl.1 $(MANDIR)/man1

help:
	mkdir $@

help/udl: udl.1 help
	nroff -man udl.1 > $@

clean:
	-$(RM) *.o *.root udl.r

clobber: clean
	-$(RM) -rf udl help

common.o:: common.h
globals.o:: common.h
