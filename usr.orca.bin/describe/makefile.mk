#
# This is the makefile for the describe(1) package.  It is for use
# with dmake(1).
#

# Use -DSTACK_CHECK in CFLAGS to show stack usage.

CFLAGS	+= -O -v -I/usr/include -s768
LDFLAGS	+= -v
LDLIBS	+= 
BINDIR	= /usr/local/bin
SBINDIR	= /usr/sbin
MANDIR	= /usr/man
INSTALL	= /bin/cp

build:  describe descc descu

descc:	descc.o basename.o descc.r
	@purge
	$(CC) $(LDFLAGS) descc.o basename.o -o $@ $(LDLIBS)
	copyfork descc.r descc

describe: describe.o basename.o describe.r
	@purge
	$(CC) $(LDFLAGS) describe.o basename.o -o $@ $(LDLIBS)
	copyfork describe.r describe

descu:	descu.o basename.o vaend.o descu.r
	@purge
	$(CC) $(LDFLAGS) descu.o basename.o vaend.o -o $@ $(LDLIBS)
	copyfork descu.r descu

descc.o:: desc.h
describe.o:: desc.h
descu.o:: desc.h

install:
	$(RM) -f /usr/local/bin/descc
	$(INSTALL) describe   $(BINDIR)
	$(INSTALL) descc      $(SBINDIR)
	$(INSTALL) descu      $(SBINDIR)
	$(INSTALL) describe.1 $(MANDIR)/man1
	$(INSTALL) descc.8    $(MANDIR)/man8
	$(INSTALL) descu.8    $(MANDIR)/man8
