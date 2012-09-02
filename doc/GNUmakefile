#
# Documentation for the GNO project (proper)
#

SRCROOT		= ..
WEB_HOME_BASE	=
SUBPROJECTS	= refs refs.aug96

include $(SRCROOT)/doc/etc/constpriv.mk
include $(SRCROOT)/doc/etc/const.mk

WEB_HOME	= $(DOCROOT_INSECURE)

# We don't have any default targets as the top level in the hierarchy
# is not maintained by this repository.
TARGETS		=

default: build

install:
	@for s in X $(SUBPROJECTS); do \
		[ "$$s" = X ] && continue; \
		[ -d "$$s" ] || continue; \
		(cd $$s; $(MAKE) $(MFLAGS) $@); \
	done

include $(SRCROOT)/doc/etc/rules.mk
include $(SRCROOT)/doc/etc/tailcat.mk

clobber::
	-/bin/rm -rf $(OUTPUT_DIR)

