#
# $Id: makefile.mk,v 1.1 1997/01/21 15:34:02 gdr Exp $
#

TARGETS	= mkso
OBJS	= mkso.o mkso.root
CFLAGS	+=

default: mkso

mkso: mkso.o
	$(CC) -o $@ $(LDFLAGS) $< $(LDLIBS)

clean clobber:
	$(RM) -f $(TARGETS) $(OBJS)
