#
# $Id: Makefile,v 1.1 1997/02/28 05:12:56 gdr Exp $
#

.INCLUDE:	../../const.mk

LIBC	:= ../../libc
LDLIBS	 = -l$(LIBC)
CFLAGS	:= -v -w -G25 $(STACK) $(DEFINES) $(INCLUDES)

default: perror

.PRECIOUS: perror.o
