#
# $Id: Makefile,v 1.2 1998/03/31 03:29:04 gdr-ftp Exp $
#

PROG		= renram5
STACK		= 768
BINDIR		= /sbin
CUSTOM_RELEASE	= true

.INCLUDE:	/src/gno/prog.mk

release: $(OBJ_DIR)$(PROG) $(PROG).8 $(DESC)
	$(INSTALL) -d $(RELBIN) $(RELMAN)/man8 $(DESC_DIR)
	$(INSTALL) $(OBJ_DIR)$(PROG) $(RELBIN)
	$(INSTALL) $(PROG).8 $(RELMAN)/man8
	$(DESCU) -o $(DESC_SRC) $(DESC_SRC) $(DESC)

install: $(OBJ_DIR)$(PROG) $(PROG).8
	$(INSTALL) -d $(BINDIR) $(MANDIR)/man8
	$(INSTALL) $(OBJ_DIR)$(PROG) $(BINDIR)
	$(INSTALL) $(PROG).8 $(MANDIR)/man8
