#
# $Id: Makefile,v 1.1 1997/10/30 04:14:21 gdr Exp $
#

# Don't name this "install"; we change the name later.
PROG		= inst

.INCLUDE:	/src/gno/paths.mk
.INCLUDE:	/src/gno/binconst.mk

BINDIR		= /usr/bin
MANDIR		= /usr/man
OBJS		= $(SRCS:s/.c/.o/:f)
OPTIMIZE	= 78
STACK		= 1024

# Add -D__STACK_CHECK__ to DEFINES to show stack usage.
DEFINES		+=
CFLAGS		+= -I/src/gno/lib/libcontrib
CFLAGS		+= -r -O$(OPTIMIZE) $(DEFINES) -s$(STACK)
LDLIBS		+= -l/src/gno/lib/libcontrib/libcontrib

RELBIN		= $(RELEASE_DIR)$(BINDIR)
RELMAN		= $(RELEASE_DIR)$(MANDIR)

INST		= ./$(PROG)

.INCLUDE:	/src/gno/binrules.mk

# Place files where they will subsequently be archived in a binary
# distribution.
#
# This is slightly different from the standard 'release' target because
# the binary is renamed when it is installed.
#
release: $(PROG) $(PROG).1 $(DESC)
	$(INST) -d $(RELBIN) $(RELMAN)/man1 $(DESC_DIR)
	$(INST) $(PROG) $(RELBIN)/install
	$(INST) $(PROG).1 $(RELMAN)/man1/install.1
	$(DESCU) -o $(DESC_SRC) $(DESC_SRC) $(DESC)
                          
install: $(PROG) $(PROG).1 $(DESC)
	$(INST) -d $(BINDIR) $(MANDIR)/man1 $(DESC_DIR)
	$(INST) $(PROG) $(BINDIR)/install
	$(INST) $(PROG).1 $(MANDIR)/man1/install.1
	$(DESCU) -o $(DESC_SRC) $(DESC_SRC) $(DESC)
