#
# Compilation constants for utilities (directories ./bin, ./sbin,
# ./usr.bin, ./usr.sbin).  These are not used when building the libraries.
#
# $Id: binconst.mk,v 1.1 1997/08/08 04:44:13 gdr Exp $
#
# Devin Reade, 1997.
#

DEFINES	+=
CFLAGS	+= -v -w
LDFLAGS	+= -v
LDLIBS	+=

COPYFORK	= /usr/orca/bin/copyfork
INSTALL		= /usr/bin/install
DESCU		= /usr/sbin/descu

# This is the created source file for the describe(1) database.
DESC_SRC	= $(RELEASE_DIR)/usr/lib/describe.src

# Define SRCS if it's not already done.
.IF $(SRCS) == $(NULL)
	SRCS	= $(PROG).c
.END

# Define DESC if it's not already done.
.IF $(DESC) == $(NULL)
	DESC	= $(PROG).desc
.END

OBJS	= $(SRCS:s/.c/.o/:f)
