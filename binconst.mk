#
# Compilation constants for utilities (directories ./bin, ./sbin,
# ./usr.bin, ./usr.sbin).  These are not used when building the libraries.
#
# $Id: binconst.mk,v 1.5 1998/02/09 08:43:44 taubert Exp $
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

# Define DESC if it's not already done.
DESC	*= $(PROG).desc

# Objects are source file names with .c changed to .o
OBJS	+= $(SRCS:s/.c/.o/:f)
