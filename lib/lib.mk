#
#	$Id: lib.mk,v 1.4 1998/02/15 19:18:20 gdr-ftp Exp $
#

.INCLUDE:	/src/gno/paths.mk
.INCLUDE:	/src/gno/lib/const.mk

# Objects are source file names with [.c|.asm] changed to .o, and placed
# in the /obj hierarchy.
OBJS	*= $(OBJ_DIR){$(SRCS:b)}.o

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

.IF $(NO_REZ) == $(NULL)
$(LIBTARGET):: $(OBJ_DIR)lib$(LIB).r
	$(CATREZ) -d $@ $(OBJ_DIR)lib$(LIB).r
.END

.INCLUDE:	/src/gno/lib/librelease.mk

# Default target for object files
$(OBJ_DIR)%.o: %.c ;	$(CC) -o $@ $(CFLAGS) -a0 -c $<
$(OBJ_DIR)%.o: %.asm ;	$(AS) -o $@ $(ASFLAGS) -a0 -c $<
$(OBJ_DIR)%.r: %.rez ;	$(REZ) -o $@ $(REZFLAGS) $<
