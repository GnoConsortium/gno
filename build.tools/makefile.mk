#
# $Id: makefile.mk,v 1.2 1997/09/21 22:52:13 gdr Exp $
#

TARGETS	= mkso
OBJS	= mkso.o mkso.root
CFLAGS	+=

default: build

build: mkso

mkso: mkso.o
	$(CC) -o $@ $(LDFLAGS) $< $(LDLIBS)

clean clobber:
	$(RM) -f $(TARGETS) $(OBJS) mkso.root

release: installbin mkso
	$(INSTALL) -d $(DISK1)
	$(INSTALL) installbin mkso $(DISK1)
