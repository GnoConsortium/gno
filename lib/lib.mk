#
#	$Id: lib.mk,v 1.2 1998/02/08 03:47:17 gdr-ftp Exp $
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

# default target
build:	$(LIBTARGET)

# Update library with out of date object files
$(LIBTARGET): $(OBJS)
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $^

.INCLUDE:	/src/gno/lib/librelease.mk

