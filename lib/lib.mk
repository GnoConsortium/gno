#
#	$Id: lib.mk,v 1.1 1998/01/26 05:49:01 taubert Exp $
#

.INCLUDE:	/src/gno/paths.mk
.INCLUDE:	/src/gno/lib/const.mk

# Objects are source file names with .c changed to .o
OBJS= $(SRCS:s/.c/.o/:f)

#
#  Check for user-specified compile/load options
#

# Was debugging requested?
.IF $(DEBUG) != $(NULL)
	CFLAGS+= -G$(DEBUG)
.END

# Was special optimizing requested?
.IF $(OPTIMIZE) == $(NULL)
	OPTIMIZE= 78
.END

# Compile and load flags passed to occ
CFLAGS+= -O$(OPTIMIZE)

# Update library with out of date object files
lib$(LIB): $(OBJS)
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $^
