#
# $Id: Makefile,v 1.2 1998/02/19 02:23:42 gdr-ftp Exp $
#

PROG	= test
SRCS	= test.c operators.c

# Use -D__STACK_CHECK__ to measure stack usage (currently requires 1280 bytes).
CFLAGS	+=
STACK	= 1280

#operators.c operators.h:	unaryop binaryop
#	$(MAKE) make_op

# use this rule to if you update binary_ops, or unary_ops
#make_op: 
#	@echo "$(MAKE) make_op invoked"
#	sh ${.CURDIR}/mkops

.INCLUDE: /src/gno/prog.mk

operators.o:	operators.h
test.o:		operators.h
