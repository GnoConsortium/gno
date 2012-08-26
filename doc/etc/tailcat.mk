#
# $Id: tailcat.mk,v 1.1 2012/08/26 02:27:36 gdr Exp $
#

$(WEB_HOME):
	mkdir -p $(WEB_HOME)

$(WEB_HOME)/%.html: %.html $(HEAD_PAGE) $(HEAD_PAGE_1) $(TAIL_PAGE) Makefile
	@echo "making $@"; \
	date="`$(DATE) < $<`"; \
	cat $(HEAD_PAGE) $(HEAD_PAGE_1) $*.html $(TAIL_PAGE) | perl -p \
		-e "s,%%DATE%%,$$date,g;" \
		-e 's,%%HTTP_SERVER%%,$(HTTP_SERVER),g;' \
		-e 's,%%HTTP_PORT%%,$(HTTP_PORT),g;' \
		-e 's,%%HTTPS_PORT%%,$(HTTPS_PORT),g;' \
		-e 's,%%FTP_SERVER%%,$(FTP_SERVER),g;' \
		-e 's,%%HTML_TITLE%%,$(HTML_TITLE),g;' \
		$(TAILCAT_LOCAL_REPLACEMENTS) \
		> $@
	@if [ "$(FIX_PERMS)" != "no" ]; then \
		chmod 644 $@; \
	fi
	@$(WEBLINT) $@
