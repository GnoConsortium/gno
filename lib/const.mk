#
# gno/lib/const.mk
#
# Compile-time configuration for the GNO libraries.  This file is only
# required for building the libraries, not for using them.
#
# Devin Reade, 1997
#
# $Id: const.mk,v 1.6 1998/02/09 08:43:58 taubert Exp $
#
# It is critical that 13/orcacdefs/defaults.h is set up correctly,
# including #defines for __appleiigs__ and __GNO__.  For the purpose
# of compiling these libraries, 13/orcacdefs should probably be otherwise
# empty.
#

DEFINES  +=
INCLUDES +=
ASFLAGS	 += -r
CFLAGS   += -r -w $(DEFINES) $(INCLUDES)
LDFLAGS	 +=
INSTALL   = /usr/bin/install
CATREZ	  = /usr/bin/catrez

# Where libraries get installed.   Most libraries should go into the
# USRLIBDIR; only "special" libraries belong in LIBDIR
LIBDIR		= /lib
USRLIBDIR	= /usr/lib
RELLIB		= $(RELEASE_DIR)$(LIBDIR)
RELUSRLIB	= $(RELEASE_DIR)$(USRLIBDIR)

# Byteworks' makelib pukes ... these object files are too much for it.
MAKELIB		= 17/makelib.apw
MAKELIBFLAGS	= -w -r -p

# This is the name of the library we're building.
LIBPFX    *= $(OBJ_DIR)
LIBTARGET  = $(LIBPFX)lib$(LIB)
