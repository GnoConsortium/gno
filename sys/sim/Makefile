#
# $Id: Makefile,v 1.3 1999/03/19 05:53:33 gdr-ftp Exp $
#

.INCLUDE: /src/gno/paths.mk
.INCLUDE: /src/gno/lib/const.mk

SIM	= $(OBJ_DIR)SIM
LIBSIM	= $(OBJ_DIR)libsim

build:	$(OBJ_DIR) $(LIBSIM) $(SIM)

$(OBJ_DIR):
	$(INSTALL) -d $@

# The 182 on the chtyp sets it to file type 0xB6; a PIF.
$(SIM):		sim.o sim.r
	$(CC) -o $@ $(OBJ_DIR)sim.o
	$(CATREZ) -d $@ $(OBJ_DIR)sim.r
	chtyp -t 182 $@

$(LIBSIM):	simlib.o simlib.r
	$(RM) -f $@
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(OBJ_DIR)simlib.o
	$(CATREZ) -d $@ $(OBJ_DIR)simlib.r

release:
	$(INSTALL) -d $(RELEASE_DIR)/usr/lib $(RELEASE_DIR)/System/System.Setup
	$(INSTALL) $(LIBSIM) $(RELEASE_DIR)/usr/lib
	$(INSTALL) $(SIM) $(RELEASE_DIR)/System/System.Setup

install:
	$(INSTALL) -d /usr/lib
	$(INSTALL) $(LIBSIM) /usr/lib

# Implicit rule to handle Rez source on case sensitive Appleshare servers
.IF $(APPLESHARE_CASE_SENSITIVE) != $(NULL)
%.r : %.rez
	$(INSTALL) $< $(TMPDIR)/$<
	$(REZ) -o $@ -c $(REZFLAGS) $(TMPDIR)/$<
	$(RM) -f $(TMPDIR)/$<
.END

# Additional dependencies
simlib.o:	simlib.asm simlib.mac simequates.equ
sim.o:		sim.asm sim.mac simequates.equ
