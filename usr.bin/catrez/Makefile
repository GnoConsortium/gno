#
# This makefile is intended for use with dmake(1).
#
# It is adapted for catrez by Dave Tribby <tribby@cup.hp.com>
# from Devin Reade's makefile for splatprep

# destination directories

BINDIR	= /usr/local/bin
MANDIR	= /usr/man


DEFINES	+= -Dcatrez
STACK	=  -s768
CFLAGS	+= $(DEFINES) $(STACK) -w -v -O
LDFLAGS	+= -v

OBJS	= catrez.o

catrez: $(OBJS) version.r
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	catrez -v -d $@ version.r

install:
	cp catrez $(BINDIR)
	cp catrez.1 $(MANDIR)/man1

clean:
	$(RM) -f *.o *.root *.r

clobber: clean
	$(RM) -f catrez
