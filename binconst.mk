#
# Compilation constants for utilities (directories ./bin, ./sbin,
# ./usr.bin, ./usr.sbin).  These are not used when building the libraries.
#
# $Id: binconst.mk,v 1.6 1998/02/15 19:43:57 gdr-ftp Exp $
#
# Devin Reade, 1997.
#

DEFINES	+=
CFLAGS	+= -w
LDFLAGS	+=
LDLIBS	+= $(LDADD:s,-l,-l/usr/lib/lib,)

# WARNING:  You *must* use descu v1.0.4 or later for these builds.
DESCU		= /usr/sbin/descu
COPYFORK	= /usr/orca/bin/copyfork
CATREZ		= /usr/bin/catrez
INSTALL		= /usr/bin/install

# $(DESC_SRC) is the created source file for the describe(1) database.
DESC_DIR	= $(RELEASE_DIR)/usr/lib
DESC_SRC	= $(DESC_DIR)/describe.src

# If no source files were defined, use program name
SRCS	*= $(PROG).c

# If no main file was defined, use program name
MAIN	*= $(PROG)
MAINSRC	*= $(MAIN).c

# Define DESC if it's not already done.
DESC	*= $(PROG).desc

# Some utils have both an original BSD man page and a GNO formatted one.
# If HAS_BSD_MANPAGE has been set, then the GNO page ends in ".1G", else
# it ends in ".1"
.IF $(HAS_BSD_MANPAGE) == $(NULL)
	MAN1SFX	= 1
.ELSE
	MAN1SFX	= 1G
.END

# Define all object files as being in the /obj hierarchy.
OBJS	*= $(OBJ_DIR){$(SRCS:b)}.o

