#
# gno/paths.mk
#
# Top-level configuration for building GNO and making a release.  This
# file is used to specify the various path names used by the build/release
# process.  It need not be shipped with a binaries-only GNO distribution.
#
# Devin Reade, 1997
#
# $Id: paths.mk,v 1.6 1998/02/15 19:44:01 gdr-ftp Exp $
#

# SRC_DIR is the top-level GNO source distribution directory (containing
# $(SRC_DIR)/gno, $(SRC_DIR)/gno/lib, and so forth).  It also corresponds
# to the top level of the CVS repository.
# I recommend using /src and defining it in /etc/namespace.
#
# OBJ_DIR can be used to store the resulting object and binary files on
# a separate partition from the source.  This is very handy when the
# source is stored on an AppleShare volume.  If you don't need this
# feature, you can either define /obj in /etc/namespace to be the same
# as /src, or simply comment out the OBJ_DIR line here.

SRC_DIR		= /src
CWD		= $(PWD:s,:,/,g)
OBJ_DIR		= /obj$(CWD:s,${SRC_DIR},,)

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

# These are the directories that will wind up becoming the distributions
# disks.  We want to be able to ship on floppy, although the preferred
# method will be via ftp.

DISKS		= $(RELEASE_DIR)/disks
DISK1		= $(DISKS)/gno.disk1
DISK2		= $(DISKS)/gno.disk2
DISK3		= $(DISKS)/gno.disk3
