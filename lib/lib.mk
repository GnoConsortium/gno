#
#	$Id: lib.mk,v 1.8 1999/03/19 05:53:33 gdr-ftp Exp $
#

.INCLUDE:	/src/gno/paths.mk
.INCLUDE:	/src/gno/lib/const.mk

# Objects are source file names with [.c|.asm] changed to .o
# If we're keeping object files on a ProDOS partition, change the
# '_' characters in file names to '.'
.IF $(PRODOS_OBJS) == true
	OBJS	+= {$(SRCS:b:s/_/./g)}.o
.ELSE
	OBJS	+= {$(SRCS:b)}.o
.END

#
#  Check for user-specified compile/load options
#

# Was debugging requested?
.IF $(DEBUG) != $(NULL)
	CFLAGS+= -G$(DEBUG)
.END

# check for a segment.
.IF $(SEGMENT) != $(NULL)
	CFLAGS+= -S $(SEGMENT)
.END

# Was special optimizing requested?
OPTIMIZE*= 78

# Compile and load flags passed to occ
CFLAGS	+= -O$(OPTIMIZE)

# build is the default target
build: $(OBJ_DIR) $(LOCAL_SETUP) $(LIBTARGET)

# create the object directory hierarchy if necessary
$(OBJ_DIR):
	$(INSTALL) -d $(OBJ_DIR)

# Update library with out of date object files
$(LIBTARGET) .PRECIOUS: $(OBJS)
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $^

.IF $(NO_REZ) == $(NULL)
$(LIBTARGET):: lib$(LIB).r
	$(CATREZ) -d $@ $(OBJ_DIR)lib$(LIB).r
.END

# Use "dmake force; dmake build" to update all the object files in a library
force: $(OBJS)
	touch $<

.INCLUDE:	/src/gno/lib/librelease.mk

# Implicit rule to handle ProDOS-renamed object files
%.o: %.O;
%.O .PRECIOUS : $$(@:b:s/./_/g).c
	$(CC) -o $(OBJ_DIR)$*.o $(CFLAGS) -c $<

# Implicit rule to handle Rez source on case sensitive Appleshare servers
.IF $(APPLESHARE_CASE_SENSITIVE) != $(NULL)
%.r : %.rez
	$(INSTALL) $< $(TMPDIR)/$<
	$(REZ) -o $@ -c $(REZFLAGS) $(TMPDIR)/$<
	$(RM) -f $(TMPDIR)/$<
.END
