#
# $Id: Makefile,v 1.3 1997/09/21 18:06:07 gdr Exp $
#

.INCLUDE:	../../../../paths.mk
.INCLUDE:	../../../const.mk

LIBC	:= ../../libc
LDLIBS	 = -l$(LIBC)
CFLAGS	:= -v -w -G25 $(STACK) $(DEFINES) $(INCLUDES)

PROGS	= buffer fdopen1 fdopen2 fgetln fgets1 filenos float1 fopen \
	  fopen2 getputch offsetof perror showfds 

OBJS	= buffer.o fdopen1.o fdopen2.o fgetln.o fgets1.o filenos.o float1.o \
		fopen.o fopen2.o getputch.o offsetof.o perror.o showfds.o 

default: $(PROGS)

.PRECIOUS: $(OBJS)

.INCLUDE:	../../rules.mk
