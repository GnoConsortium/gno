#
# Standard compilation rules for utilities (directories ./bin, ./sbin,
# ./usr.bin, ./usr.sbin).  These are not used when building the libraries.
#
# $Id: binrules.mk,v 1.1 1997/08/08 04:44:13 gdr Exp $
#
# Devin Reade, 1997.
#

build:	$(PROG)

$(PROG): $(OBJS) $(PROG).r
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LDLIBS)
	$(COPYFORK) $(PROG).r $@ -r
                
clean:
	-$(RM) -f *.o
	-$(RM) -f *.root
	-$(RM) -f *.r
	-$(RM) -f *.rej

clobber: clean
	-$(RM) $(PROG)
