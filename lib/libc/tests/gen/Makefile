#
# $Id: Makefile,v 1.2 1997/07/27 23:44:39 gdr Exp $
#

.INCLUDE:	../../../const.mk

STACK	 =
LIBC	:= ../../libc
LDLIBS	 = -l$(LIBC)
CFLAGS	:= -v -w -G25 $(STACK) $(DEFINES) $(INCLUDES)

default: \
	basename \
	dirent \
	err \
	fnmatch \
	getcwd \
	getgrent \
	getpwent \
	multiopen \
	popen \
	scandir \
	sleep \
	utime

.PRECIOUS: \
	basename.o \
	dirent.o \
	err.o \
	fnmatch.o \
	getcwd.o \
	getgrent.o \
	getpwent.o \
	multiopen.o \
	popen.o \
	scandir.o \
	sleep.o \
	utime.o
