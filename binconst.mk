#
# Compilation constants for utilities (directories ./bin, ./sbin,
# ./usr.bin, ./usr.sbin).  These are not used when building the libraries.
#
# $Id: binconst.mk,v 1.3 1997/11/01 19:10:37 gdr Exp $
#
# Devin Reade, 1997.
#

DEFINES	+=
CFLAGS	+= -w
LDFLAGS	+=
LDLIBS	+=

# WARNING:  You *must* use descu v1.0.4 or later for these builds.
DESCU		= /usr/sbin/descu
COPYFORK	= /usr/orca/bin/copyfork
CATREZ		= /usr/bin/catrez
INSTALL		= /usr/bin/install

# $(DESC_SRC) is the created source file for the describe(1) database.
DESC_DIR	= $(RELEASE_DIR)/usr/lib
DESC_SRC	= $(DESC_DIR)/describe.src

# If no source files were defined, use program name
.IF $(SRCS) == $(NULL)
	SRCS	= $(PROG).c
.END

# If no main file was defined, use program name
.IF $(MAIN) == $(NULL)
	MAIN	= $(PROG)
.END
# Define DESC if it's not already done.
.IF $(DESC) == $(NULL)
	DESC	= $(PROG).desc
.END

OBJS	= $(SRCS:s/.c/.o/:f)
