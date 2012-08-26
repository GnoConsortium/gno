#
# $Id: rules.mk,v 1.1 2012/08/26 02:27:36 gdr Exp $
#

$(WEB_HOME)/%.gif: %.gif
	install -m644 $< $@

$(WEB_HOME)/%.png: %.png
	install -m644 $< $@

$(WEB_HOME)/%.pdf: %.pdf
	install -m644 $< $@

$(WEB_HOME)/%.txt: %.txt
	install -m644 $< $@

build:	buildLocal webHome $(TARGETS) webHomePerms $(BUILD_FILES)
	@for s in X $(SUBPROJECTS); do \
		[ "$$s" = X ] && continue; \
		[ -d "$$s" ] || continue; \
		(cd $$s; $(MAKE) $(MFLAGS) $@); \
	done

buildLocal::
	@true

webHome:
	@if [ -z "$(WEB_HOME)" ]; then \
		echo "WEB_HOME not set"; \
		exit 1; \
	fi; \
	[ -d $(WEB_HOME) ] || mkdir -p $(WEB_HOME)

webHomePerms:
	@if [ -z "$(WEB_HOME)" ]; then \
		echo "WEB_HOME not set"; \
		exit 1; \
	fi; \
	find $(WEB_HOME) -type d \! -perm 0755 -exec chmod 0755 {} \; ; \
	find $(WEB_HOME) -type f \! -perm 0644 -exec chmod 644 {} \;

install::
	@/bin/rm -rf $(TARGET_DIR)
	install -d -m755 $(TARGET_DIR)
	@echo "copying files to $(TARGET_DIR)"; \
	cd $(WEB_HOME); tar -cf - . | \
	  (cd $(TARGET_DIR); tar -xpBf -);
	@echo "setting permissions on $(TARGET_DIR)"; \
	find $(TARGET_DIR) -type d \! -perm 0755 -exec chmod 0755 {} \; ; \
	find $(TARGET_DIR) -type f \! -perm 0644 -exec chmod 644 {} \;

clean::
	rm -f *~
	@for s in X $(SUBPROJECTS); do \
		[ "$$s" = X ] && continue; \
		[ -d "$$s" ] || continue; \
		(cd $$s; $(MAKE) $(MFLAGS) $@); \
	done

clobber:: clean

buildDocbookHtml:: clean
	@if [ -z "$(DOCBOOK_TOP)" ]; then \
	  echo "DOCBOOK_TOP is not set"; \
	  exit 1; \
	fi
	@htmldir="$(WEB_HOME)/html"; \
	[ -d $$htmldir ] || mkdir -p $$htmldir; \
	echo docbook2html  -o $$htmldir $(DOCBOOK_TOP); \
	docbook2html  -o $$htmldir $(DOCBOOK_TOP)
#	-cp -p *.png $(HTML_DIR)

buildDocbookPdf::
	@[ -d $(WEB_HOME) ] || mkdir -p $(WEB_HOME)
	@date
	docbook2pdf -o $(WEB_HOME) $(DOCBOOK_TOP)
	@date
