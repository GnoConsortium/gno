#
# gno/paths.mk
#
# Top-level configuration for building GNO and making a release.  This
# file is used to specify the various path names used by the build/release
# process.  It need not be shipped with a binaries-only GNO distribution.
#
# Devin Reade, 1997
#
# $Id: paths.mk,v 1.11 1999/03/19 05:53:31 gdr-ftp Exp $
#

# This one isn't really a path, but it affects the creation of path
# names.  If PRODOS_OBJS is set to "true" (minus the quotes), then the
# created object files will follow ProDOS naming conventions.

PRODOS_OBJS	= true

# Define APPLESHARE_CASE_SENSITIVE if your source exists on an Appleshare-
# served partition where the underlying filesystem is case sensitive.
# This is necessary due to the rez compiler forcing all filenames to upper
# case rather than using them as is.
#
# This is necessary (for example) for CAP and Netatalk served volumes
# where the underlying fs is ufs, or e2fs, but is not required for MACOS
# servers where the underlying fs is HFS.  Defining this macro when it is
# not needed doesn't break anything, it just makes your compilations a bit
# slower due to an extra file copy.
#
# If you're setting it here and also building any of libc, ORCALib, SysFloat,
# or SysFPEFloat, you will also want to set it in:
#	../lib/orcalibs/Source/const.mk
# 
# To turn this off, completely comment it out; don't just change the value.

# APPLESHARE_CASE_SENSITIVE = true

# SRC_DIR is the top-level GNO source distribution directory (containing
# $(SRC_DIR)/gno, $(SRC_DIR)/gno/lib, and so forth).  It also corresponds
# to the top level of the CVS repository.
# I recommend using /src and defining it in /etc/namespace.
#
# OBJ_DIR can be used to store the resulting object and binary files on
# a separate partition from the source.  This is very handy when the
# source is stored on an AppleShare volume.  If you don't need this
# feature, just define /obj in /etc/namespace to be the same as /src.
# It is not sufficient to merely change the definition of OBJ_DIR below,
# because various files in the builds assume that the /obj hierarchy
# exists.

SRC_DIR		= /src
CWD		= $(PWD:s,:,/,g)
OBJ_DIR		= /obj$(CWD:s,${SRC_DIR},,)

.SOURCE.a :	$(OBJ_DIR)
.SOURCE.o :	$(OBJ_DIR)
.SOURCE.r :	$(OBJ_DIR)
.SOURCE.root :	$(OBJ_DIR)

# RELEASE_DIR is the directory into which we will put the created
# distribution files.
# I recommend using /dist and defining it in /etc/namespace.

RELEASE_DIR	= /dist

# ORCA_DIST is the directory containing the standard Orca distribution.
# It shouldn't contain any GNO-isms.

ORCA_DIST	= /lang/orca

# ORCA_SRC is the directory containing the sources to the ORCA libraries
# This is (and should only be) used when building libc.  We do this kludge
# to avoid backward references during linking.

ORCA_SRC	= /src/lib/orcalibs/Source

# Paths to various programs
CATREZ		= /usr/bin/catrez
INSTALL		= /usr/bin/install

# Temporary directory needed in some weird cases.
.IMPORT .IGNORE: TMPDIR
.IF $(TMPDIR) == $(NULL)
TMPDIR		= 14
.END
