#
# gno/lib/libc/rules.mk
#
# Default rules for libc (override of /usr/local/lib/startup.mk).
#
# Devin Reade, 1997
#
# $Id: rules.mk,v 1.2 1997/09/05 06:48:12 gdr Exp $
#

%.o: %.c
	$(CC) -o $@ -c $(__OFLAG) $(CFLAGS) $<

%.o: %.asm
	$(AS) -o $@ -c $(__OFLAG) $(ASFLAGS) $<
	@$(RM) $*.root

#../libc: $(OBJS)
#	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(OBJS)
              
