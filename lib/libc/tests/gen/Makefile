#
# $Id: Makefile,v 1.1 1997/02/28 05:12:51 gdr Exp $
#

.INCLUDE:	../../../const.mk

STACK	 =
LIBC	:= ../../libc
LDLIBS	 = -l$(LIBC)
CFLAGS	:= -v -w -G25 $(STACK) $(DEFINES) $(INCLUDES)

default: basename dirent envtest err getcwd getgrent scandir sleep

.PRECIOUS: basename dirent.o envtest.o err.o getcwd.o getgrent.o scandir.o \
	sleep.o
