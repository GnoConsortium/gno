#
# gno/paths.mk
#
# Top-level configuration for building GNO and making a release.  This
# file is used to specify the various path names used by the build/release
# process.  It need not be shipped with a binaries-only GNO distribution.
#
# Devin Reade, 1997
#
# $Id: paths.mk,v 1.1 1997/08/08 04:44:13 gdr Exp $
#

# SRC_DIR is the top-level GNO source distribution directory (containing
# $(SRC_DIR)/gno, $(SRC_DIR)/gno/lib, and so forth).  It also corresponds
# to the top level of the CVS repository.
# I recommend using /src and defining it in /etc/namespace.

SRC_DIR		= /src

# RELEASE_DIR is the directory into which we will put the created
# distribution files.
# I recommend using /dist and defining it in /etc/namespace.

RELEASE_DIR	= /dist

# ORCA_DIST is the directory containing the standard Orca distribution.
# It shouldn't contain any GNO-isms.
# I recommend using /orca-native and defining it in /etc/namespace.

ORCA_DIST	= /orca-native

# INCLUDE_GNO is the directory containing only the header files (and subdirs
# containing header files) that will be shipped with GNO.  Normally, this
# should be $(SRC_DIR)/gno/include

INCLUDE_GNO	= $(SRC_DIR)/gno/include

# INCLUDE_ORCA is the directory containing the completely unmodified Orca/C
# header files.  (The current version is v2.1.1b2.)  If a file in here
# requires modifications, it should be copied into $(INCLUDE_GNO) and edited
# there.  Presumably, INCLUDE_ORCA is $(ORCA_DIST)/libraries/orcacdefs.

INCLUDE_ORCA	= $(ORCA_DIST)/libraries/orcacdefs

# INCLUDE_GNO_ALT is a directory residing on an HFS volume.  It is used for
# those header files which have ProDOS-incompatible file names.  These
# headers are actually "links" to the "real" headers, which reside in the
# $(INCLUDE_GNO) hierarchy.
# I recommend using /HFS-src/HFSinclude and defining /HFS-src in /etc/namespace.

INCLUDE_GNO_ALT	= /HFS-src/HFSinclude

# These are the directories that will wind up becoming the distributions
# disks.  We want to be able to ship on floppy, although the preferred
# method will be via ftp.

DISKS		= $(RELEASE_DIR)/disks
DISK1		= $(DISKS)/gno.disk1
DISK2		= $(DISKS)/gno.disk2
DISK3		= $(DISKS)/gno.disk3
