#
# Compile-time configuration for the GNO libraries.  This file is only
# required for building the libraries, not for using them.
#
# Devin Reade, 1997
#
# $Id: const.mk,v 1.1 1997/02/28 05:03:34 gdr Exp $
#
# It is critical that 13/orcacdefs/defaults.h is set up correctly,
# including #defines for __appleiigs__ and __GNO__.  For the purpose
# of compiling these libraries, 13/orcacdefs should probably be otherwise
# empty.
#
# ORCA_DIST	is a directory containing the standard Orca distribution.
#
# INCLUDE_GNO	is a directory containing only the header files (and subdirs
#		containing header files) that will be shipped with GNO.
#
# INCLUDE_ORCA	is a directory containing the completely unmodified Orca/C
#		header files.  (The current version is v2.1.1b2.)  If a
#		file in here requires modifications, it should be copied
#		into $(INCLUDE_GNO) and edited there.  Presumably,
#		INCLUDE_ORCA is $(ORCA_DIST)/libraries/orcacdefs.
#
# INCLUDE_GNO_ALT
#		is a directory residing on an HFS volume.  It is used for
#		those header files which have ProDOS-incompatible file
#		names.  These headers are actually "links" to the "real"
#		headers, which reside in the $(INCLUDE_GNO) hierarchy.
#
# I recommend using /etc/namespace to define /gno-src and /orca-native,
# but change them here if you must.
#

ORCA_DIST	= /orca-native
INCLUDE_GNO	= /bsd/include
INCLUDE_GNO_ALT	= /gno-src/HFSinclude
INCLUDE_ORCA	= $(ORCA_DIST)/libraries/orcacdefs

DEFINES  =
INCLUDES = -I$(INCLUDE_GNO) -I$(INCLUDE_ORCA) -I$(INCLUDE_GNO_ALT)
ASFLAGS	 = -r -c
CFLAGS   = -r -v -w $(DEFINES) $(INCLUDES)
LDFLAGS	 = -v

# Byteworks' makelib pukes ... these object files are too much for it.
MAKELIB		= 17/makelib.apw
MAKELIBFLAGS	= -w -r -p
