#
# Standard compilation rules for utilities (directories ./bin, ./sbin,
# ./usr.bin, ./usr.sbin).  These are not used when building the libraries.
#
# $Id: binrules.mk,v 1.2 1997/09/24 06:43:53 gdr Exp $
#
# Devin Reade, Dave Tribby, 1997.
#

# Default target, "build," generates the program file
build: $(PROG)

# Create the main program file with a ".root" and set the stack size.
# Include standard occ options
#   -a0: use .o suffix for object file
#   -c:  don't link after compiling
$(PROG).o: $(PROG).c
	$(CC) $(CFLAGS:s/ -r / /) -a0 -c $(PROG).c

# Program depends upon all the objects. Add the version resource.
$(PROG): $(OBJS) $(PROG).r
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LDLIBS)
	$(CATREZ) -d $@ $(PROG).r
                  
# Remove intermediate files.  If we don't put shell meta characters in
# here, then dmake doesn't have to start up a subshell and can instead
# exec the line directly
clean:
	-$(RM) -f $(OBJS)
	-$(RM) -f $(PROG).root
	-$(RM) -f $(PROG).r
	-$(RM) -f $(PROG).rej

# Remove intermediate files and program file
clobber: clean
	-$(RM) $(PROG)
