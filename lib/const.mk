#
# gno/lib/const.mk
#
# Compile-time configuration for the GNO libraries.  This file is only
# required for building the libraries, not for using them.
#
# Devin Reade, 1997
#
# $Id: const.mk,v 1.2 1997/09/21 06:01:12 gdr Exp $
#
# It is critical that 13/orcacdefs/defaults.h is set up correctly,
# including #defines for __appleiigs__ and __GNO__.  For the purpose
# of compiling these libraries, 13/orcacdefs should probably be otherwise
# empty.
#

DEFINES  =
INCLUDES = -I$(INCLUDE_GNO) -I$(INCLUDE_ORCA) -I$(INCLUDE_GNO_ALT)
ASFLAGS	 = -r -c
CFLAGS   = -r -w $(DEFINES) $(INCLUDES)
LDFLAGS	 =

# Byteworks' makelib pukes ... these object files are too much for it.
MAKELIB		= 17/makelib.apw
MAKELIBFLAGS	= -w -r -p
