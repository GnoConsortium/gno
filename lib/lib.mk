#
#	$Id: lib.mk,v 1.3 1998/02/09 08:44:01 taubert Exp $
#

.INCLUDE:	/src/gno/paths.mk
.INCLUDE:	/src/gno/lib/const.mk

# Objects are source file names with .c changed to .o
OBJS	+= $(SRCS:s/.c/.o/:f)

#
#  Check for user-specified compile/load options
#

# Was debugging requested?
.IF $(DEBUG) != $(NULL)
	CFLAGS+= -G$(DEBUG)
.END

# Was special optimizing requested?
OPTIMIZE*= 78

# Compile and load flags passed to occ
CFLAGS	+= -O$(OPTIMIZE)

# build is the default target
build: $(OBJ_DIR) $(LIBTARGET)

# create the object directory hierarchy if necessary
$(OBJ_DIR):
	$(INSTALL) -d $(OBJ_DIR)

# Update library with out of date object files
$(LIBTARGET) .PRECIOUS: $(OBJS)
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $^

# Use "dmake force; dmake build" to update all the object files in a library
force: $(OBJS)
	touch $<

.INCLUDE:	/src/gno/lib/librelease.mk

