#
# $Id: Makefile,v 1.2 1997/09/21 16:35:12 gdr Exp $
#

.INCLUDE:	../../../../paths.mk
.INCLUDE:	../../../const.mk

LIBC	:= ../../libc
LDLIBS	 = -l$(LIBC)
CFLAGS	:= -v -w -G25 $(STACK) $(DEFINES) $(INCLUDES)

PROGS	= buffer fdopen1 fdopen2 fgetln fgets1 filenos float1 fopen \
	  fopen2 getputch offsetof perror showfds 

default: $(PROGS)

.PRECIOUS: $(PROGS)

.INCLUDE:	../../rules.mk
