#
# $Id: Makefile,v 1.1 1997/10/30 03:35:20 gdr Exp $
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
