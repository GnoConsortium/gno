#
# $Id: Makefile,v 1.2 1997/09/21 17:54:27 gdr Exp $
#

.INCLUDE:	../../../../paths.mk
.INCLUDE:	../../../const.mk

LIBC	:= ../../libc
LDLIBS	 = -l$(LIBC)
CFLAGS	:= -v -w -G25 $(STACK) $(DEFINES) $(INCLUDES)

default: assert environ

.PRECIOUS: assert.o environ.o

.INCLUDE:	../../rules.mk
