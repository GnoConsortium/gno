#
# This makefile is for use with dmake(1).
#
# $Id: makefile.mk,v 1.2 1996/09/03 03:54:59 gdr Exp $
#

DEFINES		= -D__GNO__ # -DCHECK_STACK
INCLUDES	= -I ../contrib/src
STACK		= -s1280
CFLAGS		= $(DEFINES) $(INCLUDES) $(STACK) -w -O -v
LDFLAGS		= -v
LDLIBS		= -l ../contrib/src/lcontrib \
		  -l ../contrib/src/libc2 \
		  # -l/usr/lib/stack
BINDIR		= /usr/bin
MANDIR		= /usr/man

OBJS	= install.o

install: $(OBJS) install.r
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	copyfork install.r install -r
	@echo 'type \"dmake doinstall\" to install this program'

doinstall:
	./install -d $(BINDIR) $(MANDIR)/man1
	./install -m755 -obin -gsys ./install   $(BINDIR)
	./install -m644 -obin -gsys ./install.1 $(MANDIR)/man1

clean clobber:
	$(RM) $(OBJS) install.root install.r
