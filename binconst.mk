#
# Compilation constants for utilities (directories ./bin, ./sbin,
# ./usr.bin, ./usr.sbin).  These are not used when building the libraries.
#
# $Id: binconst.mk,v 1.11 1999/01/07 07:28:14 gdr-ftp Exp $
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
MACGEN		= 17/macgen
MACGEN_FLAGS	+= -P

# Temporary directory needed in some weird cases.
.IMPORT .IGNORE: TMPDIR
.IF $(TMPDIR) == $(NULL)
TMPDIR		= 14
.END

# $(DESC_SRC) is the created source file for the describe(1) database.
# DESC_DIR	= $(RELEASE_DIR)/usr/lib
DESC_DIR	= $(SRC_DIR)/gno/doc/describe
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
	MAN8SFX	= 8
.ELSE
	MAN1SFX	= 1G
	MAN8SFX = 8G
.END

# Objects are source file names with [.c|.asm] changed to .o
# If we're keeping object files on a ProDOS partition, change the
# '_' characters in file names to '.'
.IF $(PRODOS_OBJS) == true
	OBJS	+= {$(SRCS:b:s/_/./g)}.o
.ELSE
	OBJS	+= {$(SRCS:b)}.o
.END

# where do we put all the (renamed) mkso.data files?
MKSO_DIR	= $(RELEASE_DIR)/install
