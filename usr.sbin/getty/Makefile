#
# $Id: Makefile,v 1.2 1998/04/10 16:17:33 gdr-ftp Exp $
#

PROG	= getty
MAIN	= main
SRCS	= main.c gettytab.c init.c subr.c ttydeflts.c
LDADD	= -lutil
CFLAGS	+= -DNO_STRFTIME
STACK	= 1024		# 712 observed
BINDIR	= /usr/sbin

CUSTOM_RELEASE	= true

.INCLUDE: /src/gno/prog.mk

release: $(OBJ_DIR)$(PROG) gettytab.5G getty.8G $(DESC)
	$(INSTALL) -d $(RELBIN) $(RELMAN)/man5 $(RELMAN)/man8 $(DESC_DIR)
	$(INSTALL) $(OBJ_DIR)$(PROG) $(RELBIN)
	$(INSTALL) gettytab.5G $(RELMAN)/man5/gettytab.5
	$(INSTALL) getty.8G $(RELMAN)/man8/getty.8
	$(DESCU) -o $(DESC_SRC) $(DESC_SRC) $(DESC)

install: $(OBJ_DIR)$(PROG) gettytab.5G getty.8G
	$(INSTALL) -d $(BINDIR) $(MANDIR)/man5 $(MANDIR)/man8 $(DESC_DIR)
	$(INSTALL) $(OBJ_DIR)$(PROG) $(BINDIR)
	$(INSTALL) gettytab.5G $(MANDIR)/man5/gettytab.5
	$(INSTALL) getty.8G $(MANDIR)/man8/getty.8
