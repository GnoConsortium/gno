#
# $Id: Makefile,v 1.1 1997/10/30 04:04:34 gdr Exp $
#

# Program name
PROG	= nroff
SRCS	= nroff.c command.c escape.c io.c low.c macros.c strings.c text.c

# Add -D__STACK_CHECK__	to print, on exit, the amount of stack used
CFLAGS	+=
STACK	= 6144
# We need stack checking for now; recursion is involved.
DEBUG	= 1
BINDIR	= /usr/bin
TMACDIR	= /usr/lib/tmac
LDLIBS	= -l/lib/ltermcap

# Don't use the default 'release' and 'install' target.
CUSTOM_RELEASE	= true

.INCLUDE:	/src/gno/prog.mk

RELTMAC	= $(RELEASE_DIR)$(TMACDIR)

release: $(PROG) tmac.an tmac.s nroff.1 man.7 ms.7
	$(INSTALL) -d $(RELBIN) $(RELTMAC) $(RELMAN)/man1 $(RELMAN)/man7
	$(INSTALL) -m755 nroff $(RELBIN)
	$(INSTALL) -m644 tmac.an tmac.s $(RELTMAC)
	$(INSTALL) -m644 nroff.1 $(RELMAN)/man1
	$(INSTALL) -m644 man.7 ms.7 $(RELMAN)/man7

install: $(PROG) tmac.an tmac.s nroff.1 man.7 ms.7
	$(INSTALL) -d $(BINDIR) $(TMACDIR) $(MANDIR)/man1 $(MANDIR)/man7
	$(INSTALL) -m755 nroff $(BINDIR)
	$(INSTALL) -m644 tmac.an tmac.s $(TMACDIR)
	$(INSTALL) -m644 nroff.1 $(MANDIR)/man1
	$(INSTALL) -m644 man.7 ms.7 $(MANDIR)/man7
               
#
# additional dependancies
#
command.o::	$(SUNOS_H) nroff.h config.h
escape.o::	$(SUNOS_H) nroff.h config.h
io.o::	 	$(SUNOS_H) nroff.h config.h macros.h io.h
low.o::	 	$(SUNOS_H) nroff.h config.h
macros.o::	$(SUNOS_H) nroff.h config.h macros.h
main.o::	$(SUNOS_H) nroff.h config.h macros.h
strings.o::	$(SUNOS_H) nroff.h config.h
text.o::	$(SUNOS_H) nroff.h config.h io.h
                            
