#
# Default rules for libc/gno (override of /usr/local/lib/startup.mk)
# Devin Reade, 1997
#
# $Id: rules.mk,v 1.1 1997/02/28 05:12:39 gdr Exp $
#

%.o: %.c
	$(CC) -o $@ -c $(__OFLAG) $(CFLAGS) $<
#	$(MAKELIB) -l $(LIBC) $(MAKELIBFLAGS) $@

%.o: %.asm
	$(AS) -o $@ -c $(__OFLAG) $(ASFLAGS) $<
	@$(RM) $*.root
#	$(MAKELIB) -l $(LIBC) $(MAKELIBFLAGS) $@
