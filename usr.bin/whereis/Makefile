#
# This makefile is intended for use with dmake(1) on Apple IIGS
#
# $Id: Makefile,v 1.2 1997/10/30 04:06:11 gdr Exp $
#
# Devin Reade, September 1997
#

PROG	= whereis

BINDIR	= /usr/bin
MANDIR	= /usr/man
ETCDIR	= /etc

.INCLUDE:	/src/gno/paths.mk
.INCLUDE:	/src/gno/binconst.mk

RELBIN	= $(RELEASE_DIR)$(BINDIR)
RELMAN	= $(RELEASE_DIR)$(MANDIR)
RELETC	= $(RELEASE_DIR)$(ETCDIR)

RELINC	= $(RELEASE_DIR)/usr/include
RELLIB	= $(RELEASE_DIR)/usr/lib

OBJS		=  $(SRCS:s/.c/.o/:f)

OPTIMIZE	= 78
STACK		= 1024

# Use -D__STACK_CHECK__ to see stack usage
DEFINES		+=

INCLUDES	+= -I$(RELINC)
CFLAGS		+= -r $(INCLUDES) -O$(OPTIMIZE) $(DEFINES) -s$(STACK)

LDLIBS		+= -l$(RELLIB)/libcontrib

.INCLUDE:	/src/gno/binrules.mk

release: $(PROG) $(PROG).1 $(DESC) whereis.conf
	$(INSTALL) -d $(RELBIN) $(RELMAN)/man1 $(DESC_DIR) $(RELETC)
	$(INSTALL) $(PROG) $(RELBIN)
	$(INSTALL) $(PROG).1 $(RELMAN)/man1
	$(INSTALL) whereis.conf $(RELETC)
	$(DESCU) -o $(DESC_SRC) $(DESC_SRC) $(DESC)
