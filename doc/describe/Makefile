#
# $Id: Makefile,v 1.7 1999/07/03 16:55:41 gdr-ftp Exp $
#
# Devin Reade, January 1998
#

# What is the current version of describe, descc, and descu?  (They're
# in lockstep.)
VERSION_SHORT	= 106
VERSION_LONG	= 1.0.6


###################################################################
###### You shouldn't have to change anything below this line ######
###################################################################

# This is the describe database source file.
DB_SRC		= describe.src

# We print this in comments at the top of the generated html.
WARNING		= This is a generated file.  Do not edit.

# This is a file we generate so that we can make the indexed pages.
XREF		= xref.db

# Where, in the ftp hierarchy, will the files go?
RELDIR		= /ftp/pub/apple2/gs.specific/gno/doc

# The file which explains the db NuFX archives for the ftp site.
README_IN	= README.src.in
README_OUT	= README.src

# Files released to the ftp site.
RELEASE_README	= desc.txt
RELEASE_SRC	= desc.src.shk
RELEASE_DB	= desc.db.shk

HEAD		= head.html
TAIL		= tail.html
FINAL_DIR	= /home/gno/public_html/describe
HTML		= dbase.html \
		  index.html \
		  program.html \
		  submit.html \
		  view.html

# Directories for generated files
GEN		= $(FINAL_DIR)/gen
ENTRY		= $(FINAL_DIR)/entry

# Directory for scratch files
TDIR		= ./tmp

default:	setup xref web readme
readme:		setup $(README_OUT)
readme-install:	$(RELDIR)/$(RELEASE_README)
xref:		setup $(XREF)
release:	setup default readme-install \
			$(RELDIR)/$(RELEASE_SRC) \
			$(RELDIR)/$(RELEASE_DB)

$(XREF): $(DB_SRC)
	./mkindex < $(DB_SRC) > $(XREF)
	./mkgen $(XREF)
	chmod 644 $(GEN)/*
	chmod 644 $(ENTRY)/*

OWNER	= -ogdr -ga2ftp
MODE_F	= -m444
MODE_X	= -m775

$(RELDIR)/$(RELEASE_README):	$(README_OUT)
	install $(OWNER) $(MODE_F) $(README_OUT) $@

# We redirect nulib stdout to /dev/null because for some reason 
# it's printing out a "-^H".
$(RELDIR)/$(RELEASE_SRC):	$(DB_SRC)
	@echo "installing $@"; \
	cp $(DB_SRC) $(TDIR); \
	udl -g $(TDIR)/$(DB_SRC); \
	rm -f $@; \
	cd $(TDIR); \
	rm -f $(RELEASE_SRC); \
	nulib cf $(RELEASE_SRC) $(DB_SRC) > /dev/null; \
	install $(OWNER) $(MODE_F) $(RELEASE_SRC) $@

# We redirect nulib stdout to /dev/null because for some reason 
# it's printing out a "-^H".
$(RELDIR)/$(RELEASE_DB):	$(DB_SRC)
	@echo "installing $@"; \
	/usr/local/sbin/descc $(DB_SRC); \
	cp /usr/lib/describe.db $(TDIR); \
	cd $(TDIR); \
	NULIBOPT=type=BIN; export NULIBOPT; \
	rm -f $(RELEASE_DB); \
	nulib cf $(RELEASE_DB) describe.db > /dev/null; \
	install $(OWNER) $(MODE_F) $(RELEASE_DB) $@

web:	setup
	@tmpfile1=/tmp/desc.build.$$$$a; \
	tmpfile2=/tmp/desc.build.$$$$b; \
	trap "rm -f $$tmpfile1 $$tmpfile2" 0 1 2 15; \
	dbdate="`./getdate -describe < $(DB_SRC)`"; \
	for f in $(HTML); do \
		rm -f $$tmpfile1; \
		cat $(HEAD) $$f $(TAIL) > $$tmpfile1; \
		htmldate="`./getdate -printdate < $$tmpfile1`"; \
		rm -f $$tmpfile2; \
		sed \
			-e 's/%%WARNING%%/$(WARNING)/;' \
			-e 's/%%VERSION_SHORT%%/$(VERSION_SHORT)/;' \
			-e 's/%%VERSION_LONG%%/$(VERSION_LONG)/;' \
			-e "s/%%LAST_UPDATE%%/$$htmldate/;" \
			-e "s/%%DATE%%/$$dbdate/;" \
			< $$tmpfile1 > $$tmpfile2; \
		cmp -s $(FINAL_DIR)/$$f $$tmpfile2; \
		if [ $$? -ne 0 ]; then \
			echo "creating $(FINAL_DIR)/$$f"; \
			mv $$tmpfile2 $(FINAL_DIR)/$$f; \
			chmod 644 $(FINAL_DIR)/$$f; \
		fi; \
	done; \
	rm -f $$tmpfile1 $$tmpfile2

$(README_OUT):	$(README_IN) $(DB_SRC)
	@echo "making $@"; \
	dbdate="`./getdate -describe < $(DB_SRC)`"; \
	sed -e "s,%%DATE%%,$$dbdate,;" < $(README_IN) > $@

setup:
	@for d in $(GEN) $(ENTRY) $(FINAL_DIR) $(ENTRY) $(RELDIR) $(TDIR); do \
		[ -d $$d ] || install -d -m755 $$d; \
	done

clobber:;	rm -rf *~ $(XREF) $(TDIR) $(README_OUT)
nuke:;		rm -rf $(FINAL_DIR)/*
