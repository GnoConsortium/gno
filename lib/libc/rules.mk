#
# gno/lib/libc/rules.mk
#
# Default rules for libc (override of /usr/local/lib/startup.mk).
#
# Devin Reade, 1997
#
# $Id: rules.mk,v 1.3 1997/09/21 16:22:23 gdr Exp $
#

%.o: %.c
	$(CC) -o $@ -c $(__OFLAG) $(CFLAGS) $<

%.o: %.asm
	$(AS) -o $@ -c $(__OFLAG) $(ASFLAGS) $<
	@$(RM) $*.root

../libc .PHONY: $(OBJS)
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(OBJS)
              
