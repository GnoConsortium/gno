#
# This file is intended for use with dmake
#
# $Id: makefile.mk,v 1.1 1996/02/10 08:27:30 gdr Exp $
#
# VAFLAGS must use an optimization level of at least -O8, and no
# -g or -G* flag
#
# Include -DCHECK_STACK in DEFINES to get stack usage and debug info.
# If you use -DCHECK_STACK, you will also have to add the stack checking
# library.  I keep mine in "/usr/lib/stack".
#

DEFINES	= -Dlint -DGNO
STACK	= -s1280
CFLAGS	+= $(DEFINES) $(STACK) -w -O
VAFLAGS	+= $(DEFINES) $(STACK) -w -O
LDFLAGS	+= -v
LDLIBS	=
OBJS	= test.o    test2.o    operators.o
ROOTS	= test.root test2.root operators.root

test:	$(OBJS) test.r
	@purge
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	copyfork test.r test -r

test.o:		test.c operators.h
	@purge
	$(CC) -c $(CFLAGS) -o $@ test.c

test2.o:	test2.c
	@purge
	$(CC) -c $(VAFLAGS) -o $@ test2.c

operators.o:	operators.c operators.h
	@purge
	$(CC) -c $(CFLAGS) -o $@ operators.c

operators.c operators.h:	unaryop binaryop
	$(MAKE) make_op

clean:
	$(RM) -f $(OBJS) $(ROOTS) test.r

clobber: clean
	$(RM) -f test

# use this rule to if you update binary_ops, or unary_ops
make_op: 
	@echo "$(MAKE) make_op invoked"
#	sh ${.CURDIR}/mkops
