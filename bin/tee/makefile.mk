#
# This makefile is intended for use with dmake(1)
#
# $Id: makefile.mk,v 1.2 1996/09/03 03:56:06 gdr Exp $
#

INSTALL	= /usr/bin/install
BINDIR	= /bin
MANDIR	= /man/man1

DEFINES	= -D_POSIX_SOURCE

# CFLAGS	= -w -i -G25 -v -DCHECK_STACK=1 $(DEFINES)
# LDFLAGS	= -l/usr/lib/gnulib -l/usr/lib/stack

CFLAGS	= -w -i -O -s768 $(DEFINES)
LDFLAGS	= -l/usr/lib/gnulib -s768

tee: tee.o tee.r
	$(CC) $(LDFLAGS) tee.o $(LDLIBS) -o $@
	copyfork tee.r tee -r

install:
	$(INSTALL) -m755 -obin -gsys -d $(BINDIR) $(MANDIR)
	$(INSTALL) -m755 -obin -gsys tee   $(BINDIR)
	$(INSTALL) -m644 -obin -gsys tee.1 $(MANDIR)

clean clobber:
	$(RM) tee.r tee.o tee.root
