#
# gno/usr.bin/false/Makefile
#
# Devin Reade, 1997.
#
# $Id: Makefile,v 1.1 1997/08/08 04:45:41 gdr Exp $
#

.INCLUDE:	../../paths.mk

# The name of the program.

PROG	= false

# The list of source files.  The first one must contain main().  This
# is optional if and only if there is one source file that has a name
# matching $(PROG).c
# SRCS	= false.c

# The name of the describe entry.  Defaults to $(PROG).desc
# DESC	= false.desc

.INCLUDE:	../../binconst.mk

# Add to some of the constants:
DEFINES		+=
CFLAGS		+= $(DEFINES)
LDFLAGS		+=
LDLIBS		+=

# Import the standard rules for making targets.
.INCLUDE:	../../binrules.mk

#
# The rest of this file is component-specific.
#
BINDIR		= /bin
MANDIR		= /usr/man

RELBIN	= $(RELEASE_DIR)$(BINDIR)
RELMAN	= $(RELEASE_DIR)$(MANDIR)

# Place files where they will subsequently be archived in a binary
# distribution.
release:
	$(INSTALL) -d $(RELBIN) $(RELMAN)/man1
	$(INSTALL) $(PROG) $(RELBIN)
	$(INSTALL) $(PROG).1 $(RELMAN)/man1
	$(DESCU) $(DESC_SRC) $(DESC) > /tmp/describe.src
	$(MV) /tmp/describe.src $(DESC_SRC)

#
# Additional dependancies
#

# false.o::	another_included_header.h
