#
# $Id: Makefile,v 1.1 1997/12/19 07:54:49 gdr Exp $
#
# Devin Reade, December 1997

build: nullport

nullport: port.o
	$(CC) -o $@ $(LDFLAGS) $< $(LDLIBS)
	chtyp -t187 -a2 $@
# 187 == 0xBB
