#
# Makefile for the man package, for use with dmake(1).
#

# Location for executables.  They should normally be /usr/sbin and /usr/bin.

SBINDIR	= /usr/sbin
BINDIR	= /usr/bin

# Location for man pages.  Usually /usr/man.

MANDIR	= /usr/man

#
# You should not have to change anything below this line
#
# Define:	-DDEBUG to produce more debugging info and checks
#
#		-DSTACK_CHECK to show stack usage.  If you use this
#		one, ensure you add -l/usr/lib/stack to your LDLIBS.
#

STACK		= -s1270
DEFINES		=
CFLAGS		= $(DEFINES) $(STACK) -w -v -O -I/usr/include
LDFLAGS		= -v      
LDLIBS		= -l/usr/lib/gnulib -l/usr/lib/stack
CP		= /bin/cp -f

build: apropos catman makewhatis man whatis

apropos: apropos.o apropos2.o util.o utilgs.o globals.o
	$(CC) $(LDFLAGS) $< $(LDLIBS) -o $@

catman: catman.o util.o utilgs.o globals.o common.o
	$(CC) $(LDFLAGS) $< $(LDLIBS) -o $@

makewhatis: makewhatis.o fillbuffer.o process.o
	$(CC) $(LDFLAGS) $< $(LDLIBS) -o $@

man: man.o man2.o apropos2.o util.o utilgs.o globals.o common.o
	$(CC) $(LDFLAGS) $< $(LDLIBS) -o $@

whatis: whatis.o apropos2.o util.o utilgs.o globals.o
	$(CC) $(LDFLAGS) $< $(LDLIBS) -o $@

clobber:                                 
	$(RM) *.o *.root

install:
	$(CP) apropos man whatis $(BINDIR)
	$(CP) catman makewhatis $(SBINDIR)
	$(CP) apropos.1 man.1 whatis.1 $(MANDIR)/man1
	$(CP) catman.8 makewhatis.8 $(MANDIR)/man8

# additional dependancies       

apropos.o:: man.h util.h
apropos2.o:: man.h util.h
catman.o:: man.h util.h
common.o:: man.h util.h
fillbuffer.o:: makewhatis.h
globals.o:: man.h
makewhatis.o:: makewhatis.h
man.o:: man.h util.h
man2.o:: man.h util.h
process.o:: makewhatis.h
util.o:: util.h
utilgs.o:: util.h
whatis.o:: man.h util.h
