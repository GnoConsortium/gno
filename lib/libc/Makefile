#
# This is the top-level makefile for libc, GNO v2.0.6
#
# dmake is too bloated right now to due recursive makes well, so before
# you run make in this directory, ensure you run make in the following
# directories:
#		gen, gno, locale, stdio, stdlib, string, sys
#
# $Id: Makefile,v 1.3 1997/07/27 23:59:49 gdr Exp $
#

.INCLUDE:	../const.mk

LIBC_OBJ	= \
	sys/exec.o \
	sys/syscall.o \
	sys/trap.o \
	gno/gnocmd.o \
	gno/gnomisc.o \
	gno/gsstring.o \
	gno/map.o \
	gno/parsearg.o \
	gno/stack.o \
	gen/basename.o \
	gen/bmem.o \
	gen/compat.o \
	gen/crypt.o \
	gen/crypta.o \
	gen/dirent.o \
	gen/err.o \
	gen/fnmatch.o \
	gen/fts.o \
	gen/getcwd.o \
	gen/getgrent.o \
	gen/getpass.o \
	gen/getpwent.o \
	gen/getttyent.o \
	gen/hostname.o \
	gen/oldlog.o \
	gen/popen.o \
	gen/pwcache.o \
	gen/scandir.o \
	gen/setjmp.o \
	gen/sleep.o \
	gen/syslog.o \
	gen/tty.o \
	gen/utime.o \
	locale/table.o \
	stdio/fdopen.o \
	stdio/fgetln.o \
	stdio/mktemp.o \
	stdio/perror.o \
	stdio/tempnam.o \
	stdlib/environ.o \
	stdlib/getopt.o \
	stdlib/getsubopt.o \
	string/case.o \
	string/str.o

libc:	$(LIBC_OBJ)
	$(RM) -f $@
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(LIBC_OBJ)
