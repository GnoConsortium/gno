#
# This is the makefile for the describe(1) package.  It is for use
# with dmake(1).
#

# Use -DSTACK_CHECK in CFLAGS to show stack usage.

CFLAGS	+= -O -w -v -I/usr/include -s768
LDFLAGS	+= -v
LDLIBS	+= -l/usr/lib/lgetopt -l/usr/lib/stack
BINDIR	= /usr/local/bin
SBINDIR	= /usr/sbin
MANDIR	= /usr/man
INSTALL	= /bin/cp

build:  describe descc descu

descc:	descc.o basename.o
	@purge
	$(CC) $(LDFLAGS) $< -o $@ $(LDLIBS)

describe: describe.o basename.o
	@purge
	$(CC) $(LDFLAGS) $< -o $@ $(LDLIBS)

descu:	descu.o basename.o
	@purge
	$(CC) $(LDFLAGS) $< -o $@ $(LDLIBS)

basename.o: basename.c
	$(CC) -c $(CFLAGS) basename.c

descc.o: descc.c desc.h
	$(CC) -c $(CFLAGS) descc.c

describe.o: describe.c desc.h
	$(CC) -c $(CFLAGS) describe.c

descu.o: descu.c desc.h
	$(CC) -c $(CFLAGS) descu.c

install:
	$(RM) -f /usr/local/bin/descc
	$(INSTALL) describe   $(BINDIR)
	$(INSTALL) descc      $(SBINDIR)
	$(INSTALL) descu      $(SBINDIR)
	$(INSTALL) describe.1 $(MANDIR)/man1
	$(INSTALL) descc.8    $(MANDIR)/man8
	$(INSTALL) descu.8    $(MANDIR)/man8
