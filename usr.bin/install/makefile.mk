#
# This makefile is for use with dmake(1).
#
# $Id: makefile.mk,v 1.1 1996/03/31 23:38:34 gdr Exp $
#

DEFINES		= -DCHECK_STACK
STACK		= -s1280
MAINFLAGS	= $(DEFINES) $(STACK) -w -O
CFLAGS		= $(DEFINES) $(STACK) -w -O -r
LDFLAGS		= -v
LDLIBS		= -l/usr/lib/stack
BINDIR		= /usr/bin
MANDIR		= /usr/man

OBJS	= install.o basename.o c2gs.o copyfile.o errnoGS.o \
	  expandpath.o stringGS.o

install: $(OBJS) install.r
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	copyfork install.r install -r
	@echo 'type \"dmake doinstall\" to install this program'

install.o: install.c install.h
	$(CC) -c $(MAINFLAGS) install.c -o $@

doinstall:
	./install -m755 -obin -gsys ./install   $(BINDIR)
	./install -m644 -obin -gsys ./install.1 $(MANDIR)/man1

clean clobber:
	$(RM) $(OBJS) install.root install.r

basename.o	:: install.h
c2gs.o		:: install.h
copyfile.o	:: install.h
errnoGS.o	:: install.h
expandpath.o	:: install.h
stringGS.o	:: install.h
                                    
