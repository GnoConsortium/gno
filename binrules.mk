#
# Standard compilation rules for utilities (directories ./bin, ./sbin,
# ./usr.bin, ./usr.sbin).  These are not used when building the libraries.
#
# $Id: binrules.mk,v 1.4 1998/02/09 08:43:46 taubert Exp $
#
# Devin Reade, Dave Tribby, 1997.
#

# Default target, "build," generates the program file
build: $(OBJ_DIR) $(OBJ_DIR)$(PROG)

# create the object directory hierarchy if necessary
$(OBJ_DIR):
	$(INSTALL) -d $(OBJ_DIR)

# Create the main program file with a ".root" and set the stack size.
# Include standard occ options
#   -a0: use .o suffix for object file
#   -c:  don't link after compiling
$(MAIN).o: $(MAIN).c
	$(CC) -o $@ $(CFLAGS:s/ -r / /) -a0 -c $(MAIN).c

# Program depends upon all the objects. Add the version resource.
$(OBJ_DIR)$(PROG): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $< $(LDLIBS)
	$(CATREZ) -d $@ $(OBJ_DIR)$(PROG).r
$(OBJ_DIR)$(PROG): $(PROG).r
                  
# Remove intermediate files.  If we don't put shell meta characters in
# here, then dmake doesn't have to start up a subshell and can instead
# exec the line directly
clean:
	-$(RM) -f $(OBJ_DIR){$(OBJS)}
	-$(RM) -f $(OBJ_DIR)$(MAIN).root
	-$(RM) -f $(OBJ_DIR)$(PROG).r
	-$(RM) -f $(PROG).rej

# Remove intermediate files and program file
clobber: clean
	-$(RM) $(OBJ_DIR)$(PROG)
