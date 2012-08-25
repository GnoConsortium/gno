#
# $Id: GNUmakefile,v 1.1 2012/08/25 07:21:59 gdr Exp $
#

SRCROOT = ..

include $(SRCROOT)/etc/constpriv.mk
include $(SRCROOT)/etc/const.mk

INSTALL_REAL	= $(GNO_PUBLIC_HTML)/refs
INSTALL_FAKE	= $(INSTALL_REAL)-test

HTML_ROOT	= http://www.gno.org/gno/
FTP_ROOT	= ftp://ftp.gno.org/pub/apple2/gs.specific/gno
GIFDIR		= $(HTML_ROOT)icons
FAQ		= $(HTML_ROOT)FAQ.html
FAQLOG		= $(HTML_ROOT)FAQ-log.html
FAQ_TEXT	= $(FTP_ROOT)/doc/faq/CURRENT
FAQLOG_TEXT	= $(FTP_ROOT)/doc/faq/LOG
FTP_INTRO	= $(FTP_ROOT)/doc/refs/intro
FTP_KERN	= $(FTP_ROOT)/doc/refs/kern
FTP_GSH		= $(FTP_ROOT)/doc/refs/gsh
OLD_HTML	= $(HTML_ROOT)docs.Aug96

# Pathnames for various programs.
DVIPS		= dvips
LATEX		= latex
TEX2HTML	= latex2html
MAKEINDEX	= makeindex
GETDATE		= ./getdate -date
GETVERS		= ./getdate -version

# Where do we build our pages?  HTMLD is for latex-generated html.
BUILDDIR	= ./work

DVI		= intro.dvi    kern.dvi
PS_LTR		= intro.ltr.ps kern.ltr.ps # gsh-ltr-ps
PS_A4		= intro.a4.ps  kern.a4.ps

REF_PAGES	= $(BUILDDIR)/intro.html \
		  $(BUILDDIR)/kern.html
#		  gsh-h

# These are no longer published from this repo
MAIN_PAGES	= $(BUILDDIR)/index.html \
		  $(BUILDDIR)/faqindex.html \
		  $(BUILDDIR)/refs.html \
		  $(BUILDDIR)/manindex.html \
		  $(BUILDDIR)/related.html \
		  $(BUILDDIR)/oldrefs.html \
		  $(BUILDDIR)/bugs.html \
		  $(BUILDDIR)/unaval.html

ICONS		=
ICONS2		= $(BUILDDIR)/icons/back.gif \
		  $(BUILDDIR)/icons/a2viewable.gif

STATUS_PAGES	= $(BUILDDIR)/status-bin.html \
		  $(BUILDDIR)/status-bin.txt \
		  $(BUILDDIR)/status-lib.txt

# Format of the comment block for each html page.
CONTACT1	= '$(NAME)\
	<A HREF="mailto:$(ADDRESS)" TITLE="GNO Documentation Feedback">\
	&lt;$(ADDRESS)&gt;</A><br>Last Updated: '
CONTACT2	= '<br>Document Version: '

# Other variables for building things
DVIPS_FLAGS	=
LATEX_FLAGS	=
TEX2HTML_FLAGS	= -info "" -local_icons -dir $(BUILDDIR) -up_url refs.html \
		  -up_title "GNO Reference Manuals"
TEXINPUTS	= ':/usr/local/lib/latex2html'

# Scratch files to clean up
MANUAL_GARBAGE	= 
TEX_GARBAGE	= *.err *.aux *.log *.dvi *.toc *.idx *.ilg *.ind
DVIPS_GARBAGE	= *.ps *.djc
BIB_GARBAGE	= *.blg *.bbl

#build:	prep icons dvi ps html
build:	prep dvi ps html

install: clobber_install work_install ps_install

clobber_install:
	-/bin/rm -rf $(INSTALL_REAL)/*

fake-install:
	$(MAKE) install INSTALL_REAL=$(INSTALL_FAKE)

icons:	$(ICONS)

# html:	$(MAIN_PAGES) $(REF_PAGES) $(STATUS_PAGES)
html:	$(REF_PAGES) $(STATUS_PAGES)
dvi:	$(DVI)
ps:	ps_ltr ps_a4
ps_ltr:	$(PS_LTR)
ps_a4:	$(PS_A4)

#
# Install targets
#
ps_install:
	@echo "**** not yet doing ps_install"

work_install:
	@if [ ! -d $(INSTALL_REAL) ]; then \
		echo "$(INSTALL_REAL): no such directory"; \
		exit 1; \
	fi
	@workdirs=`(cd work; find . -type d -print)`; \
	for d in $$workdirs; do \
		if [ ! -d $(INSTALL_REAL)/$$d ]; then \
			echo "install -d -m755 $(INSTALL_REAL)/$$d"; \
			install -d -m755 $(INSTALL_REAL)/$$d; \
		fi; \
	done
	@workfiles=`(cd work; find . -type f \( -name '*.html' -o -name '*.txt' -o -name '*.png' -o -name '*.css' -o -name '*.jpg' -o -name '*.gif' \) -print)`; \
	for f in $$workfiles; do \
		if [ ! -d $(INSTALL_REAL)/$$f ]; then \
			echo "install -m644 ./work/$$f $(INSTALL_REAL)/$$f"; \
			install -m644 ./work/$$f $(INSTALL_REAL)/$$f; \
		fi; \
	done

#
# Preparation targets
#
prep:	$(BUILDDIR) $(BUILDDIR)/webconfig.pl $(BUILDDIR)/newer
#       $(BUILDDIR)/icons

$(BUILDDIR):
	@[ -d $(BUILDDIR) ] || mkdir $(BUILDDIR)

$(BUILDDIR)/icons:
	@[ -d $(BUILDDIR)/icons ] || mkdir $(BUILDDIR)/icons

$(BUILDDIR)/newer: newer.c
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $< $(LDLIBS)

$(BUILDDIR)/webconfig.pl:
	@echo "creating $@"
	@rm -f $@
	@echo '$$htmlroot = "$(HTML_ROOT)";' >> $@
	@echo '$$ftproot = "$(FTP_ROOT)";' >> $@
	@echo '$$ftp_intro = "$(FTP_INTRO)";' >> $@
	@echo '$$ftp_kern = "$(FTP_KERN)";' >> $@
	@echo '$$ftp_gsh = "$(FTP_GSH)";' >> $@
	@echo '$$faq = "$(FAQ)";' >> $@
	@echo '$$faqlog =  "$(FAQLOG)";' >> $@
	@echo '$$faq_text = "$(FAQ_TEXT)";' >> $@
	@echo '$$faqlog_text = "$(FAQLOG_TEXT)";' >> $@
	@echo '$$old_html = "$(OLD_HTML)";' >> $@
	@echo '$$gifdir = "$(GIFDIR)";' >> $@
	@echo '$$contact'" = '$(ADDRESS)';" >> $@

#
# Main page build targets
#
$(BUILDDIR)/index.html: index.html tail.html
	./replace index.html tail.html > $@

$(BUILDDIR)/faqindex.html: faqindex.html head.html tail.html
	./replace -title='GNO Frequently Asked Questions (Index)' \
		head.html $< tail.html > $@

$(BUILDDIR)/refs.html: refs.html head.html tail.html
	./replace -title='GNO Reference Manuals' \
		head.html $< tail.html > $@

$(BUILDDIR)/manindex.html: manindex.html head.html tail.html
	./replace -title='GNO Manual Pages Index' \
		head.html $< tail.html > $@

$(BUILDDIR)/related.html: related.html head.html tail.html
	./replace -title='GNO-Related Web Pages' \
		head.html $< tail.html > $@

$(BUILDDIR)/bugs.html: bugs.html head.html tail.html
	./replace -title='GNO Bug Reports' \
		head.html $< tail.html > $@

$(BUILDDIR)/oldrefs.html: oldrefs.html head.html tail.html
	./replace -title='Old (v2.0.4) GNO Documentation' \
		head.html $< tail.html > $@

$(BUILDDIR)/unaval.html: unaval.html head.html tail.html
	./replace -title='Unpublished Document' \
		head.html $< tail.html > $@

# This target creates the html versions of all the current man pages,
# plus the chapter index pages.  Unfortunately, this target rebuilds
# _all_ of it's files, not just those that are out of date.
man:	$(BUILDDIR)/newer
	@GNOROOT=../.. MANHTML=$(MANHTML) NAME="$(NAME)" \
	ADDR="$(ADDRESS)" MKSO="$(MKSO)" HTMLROOT="$(HTML_ROOT)" \
	./mkhtmlman

# looks like newer versions of latex don't need this hack anymore
cleanhtml:
	@echo "cleaning guano from html files"; \
	perl -pi \
		-e 's/NOWRAP//g;' \
		-e '(/<!--\s*cleantable-start\s*-->/) && ($$cts=1);' \
		-e '(/<!--\s*cleantable-end\s*-->/) && ($$cts=0);' \
		-e '($$cts == 1) && (s/(<BR>|<HR>)//g);' \
		$(BUILDDIR)/*.html

clean:
	-rm -rf *~ $(TEX_GARBAGE) $(BIB_GARBAGE) $(DVIPS_GARBAGE) $(UPLOADDIR)
#	cd gsh; $(MAKE) $@

clobber: clean
	-rm -rf $(MANUAL_GARBAGE) $(BUILDDIR)
#	-rm -rf $(MANUAL_GARBAGE) $(HTMLD) $(PSD) $(DVID) newer
#	cd gsh; $(MAKE) $@

#
# Status page build targets
#
$(BUILDDIR)/status-bin.html: ../../NOTES/status.bin
	./mkstatus $< $@
	chmod 644 $@

$(BUILDDIR)/status-bin.txt: ../../NOTES/status.bin
	install -m644 $< $@

# The lib version doesn't quite have the right format
#$(BUILDDIR)/status-lib.html: ../../NOTES/status.lib
#	./mkstatus $< $@
#	chmod 644 $@

$(BUILDDIR)/status-lib.txt: ../../NOTES/status.lib
	install -m644 $< $@

#
# Icon rules
#
$(BUILDDIR)/icons/back.gif: icons/back.gif
	install -m644 $< $@

$(BUILDDIR)/icons/a2viewable.gif: icons/a2viewable.gif
	install -m644 $< $@

#
# Implicit rules
#
$(BUILDDIR)/%.html: %.tex
	@$(TEX2HTML) \
	-prefix "$*". -no_auto_link \
	-address $(CONTACT1)"`$(GETDATE) <$<`"$(CONTACT2)"`$(GETVERS) <$<`" \
	-up_title \
	"`perl -n -e '(m,<title>(.*)</title>,) && printf("%s\n",$$1);'\
	< $(BUILDDIR)/refs.html `" \
	$(TEX2HTML_FLAGS) $<

%.ltr.ps: %.dvi
	$(DVIPS) -tletter $(DVIPS_FLAGS) -o $@ $<

%.a4.ps: %.dvi
	$(DVIPS) -ta4 $(DVIPS_FLAGS) -o $@ $<

%.dvi: %.tex
	@for i in 1 2 3; do \
		echo "****** latex pass $$i"; \
		TEXINPUTS=$(TEXINPUTS) $(LATEX) $(LATEX_FLAGS) $<; \
		if [ "$*" = intro ]; then \
			echo "****** running bibtex"; \
			bibtex $*; \
		fi; \
		$(MAKEINDEX) $*; \
	done; \
	echo "****** latex passes done"
