#
# $Id: Makefile,v 1.2 1998/03/08 18:18:57 gdr-ftp Exp $
#

# Don't name this "install"; we change the name later.
PROG		= inst
BINDIR		= /usr/bin
CUSTOM_RELEASE	= true
LDADD		= -lcontrib
OPTIMIZE	= 78
STACK		= 1024

.INCLUDE:	/src/gno/prog.mk

INST		= $(OBJ_DIR)$(PROG)

# This is and the next target are slightly different from the standard
# 'release' and 'install' targets because the binary is renamed when
# it is installed.

release: $(OBJ_DIR)$(PROG) $(PROG).$(MAN1SFX) $(DESC)
	$(INST) -d $(RELBIN) $(RELMAN)/man1 $(DESC_DIR)
	$(INST) $(OBJ_DIR)$(PROG) $(RELBIN)/install
	$(INST) $(PROG).$(MAN1SFX) $(RELMAN)/man1/install.1
	$(DESCU) -o $(DESC_SRC) $(DESC_SRC) $(DESC)

install: $(OBJ_DIR)$(PROG) $(PROG).$(MAN1SFX)
	$(INST) -d $(BINDIR) $(MANDIR)/man1
	$(INST) $(OBJ_DIR)$(PROG) $(BINDIR)/install
	$(INST) $(PROG).$(MAN1SFX) $(MANDIR)/man1/install.1
