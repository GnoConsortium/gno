#
# Makefile for udl
# Copyright (c) 1993-1994 Soenke Behrens
# For use with dmake
#
# $Id: Makefile.gs,v 1.4 1995/02/08 05:25:09 gdr Exp $
#
# Define the following as necessary:
#
#	HAS_ATEXIT if your system has atexit()
#
#	_POSIX_C_SOURCE and _POSIX_SOURCE if your compiler is Posix compliant
#
#	READDIR_RETURNS_DOT if your direct readdir() function will return
#	entries for "." and "..".  SunOS4 is known to do this.
#
#	BROKEN_REALLOC if your realloc() doesn't behave like malloc() when
#	passed a NULL pointer as the first argument.
#
#	GNO if you are compiling on the IIgs.  This will allow for both
#	':' and '/' as pathname separators.
#
#	OVERFLOW_CHECK  Udl uses one recursive subroutine.  Define this if
#	you want to check for stack overflows for this routine (independent
#	of any compiler flags).  Strongly recommended.
#
#	CHECK_STACK if you want stack usage to be displayed (IIgs only).
#	You will also have to specify -l/usr/lib/stack in LDFLAGS.

DEFINES = -DGNO -D_POSIX_C_SOURCE -D_POSIX_SOURCE -DHAS_ATEXIT \
	  -DOVERFLOW_CHECK
#CFLAGS	= $(DEFINES) -O -w -v -s2048
#LDFLAGS	= -v -l/usr/lib/gnulib -s2048
CFLAGS	= $(DEFINES) -O -v -s2048
LDFLAGS	= -v -s2048

#
# You should not have to modify anything beyond this point
#

udl: udl.o udluse.o udl.r common.o globals.o
#	cp udl.r udl
	$(CC) $(LDFLAGS) -o udl udl.o udluse.o common.o globals.o

udl.o: udl.gs.c common.h
	$(CC) -c $(CFLAGS) -o udl.o udl.gs.c

install:
	cp -f udl /usr/local/bin
	cp -f udl.1 /usr/man/man1

docs: udl.1
	nroff -man udl.1 >help/udl

clean:
	$(RM) *.o udl

dist:
	@echo "Sorry, automatic packing not supported yet"

common.o:: common.h
globals.o:: common.h
