#
# $Id: Makefile,v 1.1 1997/02/28 05:12:54 gdr Exp $
#

.INCLUDE:	../../const.mk

LIBC	:= ../../libc
LDLIBS	 = -l$(LIBC)
CFLAGS	:= -v -w -G25 $(STACK) $(DEFINES) $(INCLUDES)

TARGETS	= gnomisc   gsstring
OBJS	= gnomisc.o gsstring.o

default: $(TARGETS)

.PRECIOUS: $(OBJS)

clobber clean:
	$(RM) -f $(TARGETS) $(OBJS)
