#
# gno/lib/const.mk
#
# Compile-time configuration for the GNO libraries.  This file is only
# required for building the libraries, not for using them.
#
# Devin Reade, 1997
#
# $Id: const.mk,v 1.4 1997/12/21 20:04:19 gdr Exp $
#
# It is critical that 13/orcacdefs/defaults.h is set up correctly,
# including #defines for __appleiigs__ and __GNO__.  For the purpose
# of compiling these libraries, 13/orcacdefs should probably be otherwise
# empty.
#

DEFINES  +=
INCLUDES +=
ASFLAGS	 += -r -c
CFLAGS   += -r -w $(DEFINES) $(INCLUDES)
LDFLAGS	 +=
INSTALL   = /usr/bin/install
CATREZ	  = /usr/bin/catrez

# Byteworks' makelib pukes ... these object files are too much for it.
MAKELIB		= 17/makelib.apw
MAKELIBFLAGS	= -w -r -p
