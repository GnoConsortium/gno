#
# $Id: Makefile,v 1.1 1998/01/14 05:10:01 gdr Exp $
#
# Devin Reade, January 1998
#

# What is the current version of describe, descc, and descu?  (They're
# in lockstep.)
VERSION_SHORT	= 103
VERSION_LONG	= 1.0.3


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
FINAL_HTML	= $(FINAL_DIR)/dbase.html \
		  $(FINAL_DIR)/index.html \
		  $(FINAL_DIR)/program.html \
		  $(FINAL_DIR)/submit.html \
		  $(FINAL_DIR)/view.html

# Directories for generated files
GEN		= $(FINAL_DIR)/gen
ENTRY		= $(FINAL_DIR)/entry

# Directory for scratch files
TDIR		= ./tmp

default:	xref web readme
web:		$(FINAL_DIR) $(FINAL_HTML)
readme:		$(README_OUT)

xref:	$(GEN) $(ENTRY)
	./mkindex < $(DB_SRC) > $(XREF)
	./mkgen $(XREF)
	chmod 644 $(GEN)/*
	chmod 644 $(ENTRY)/*

OWNER	= -ogdr -ga2ftp
MODE_F	= -m444
MODE_X	= -m775

release:	$(RELDIR)/$(RELEASE_README) $(RELDIR)/$(RELEASE_SRC)

$(RELDIR)/$(RELEASE_README):	$(RELDIR) $(README_OUT)
	install $(OWNER) $(MODE_F) $(README_OUT) $@

$(RELDIR)/$(RELEASE_SRC):	$(RELDIR) $(TDIR) $(DB_SRC)
	@echo "installing $@"; \
	cp $(DB_SRC) $(TDIR); \
	udl -g $(TDIR)/$(DB_SRC); \
	rm -f $@; \
	cd $(TDIR); \
	nulib cf $(RELEASE_SRC) $(DB_SRC); \
	install $(OWNER) $(MODE_F) $(RELEASE_SRC) $@

$(RELDIR):;	@[ -d $(RELDIR) ] || install -d $(OWNER) $(MODE_X) $(RELDIR)
$(TDIR):;	@[ -d $(TDIR) ] || install -d $(OWNER) $(MODE_X) $(TDIR)

$(FINAL_DIR):;	@[ -d $(FINAL_DIR) ]	|| install -d -m755 $(FINAL_DIR)
$(GEN):;	@[ -d $(GEN) ]		|| install -d -m755 $(GEN)
$(ENTRY):;	@[ -d $(ENTRY) ]	|| install -d -m755 $(ENTRY)

#
# Implicit rules.
#

$(FINAL_DIR)/%.html: %.html
	@echo "creating $@"; \
	tmpfile=/tmp/desc.build.$$; \
	trap "rm -f $$tmpfile" 0 1 2 15; \
	cat $(HEAD) $< $(TAIL) > $$tmpfile; \
	htmldate="`./getdate -printdate < $$tmpfile`"; \
	dbdate="`./getdate -describe < $(DB_SRC)`"; \
	sed \
		-e 's/%%WARNING%%/$(WARNING)/;' \
		-e 's/%%VERSION_SHORT%%/$(VERSION_SHORT)/;' \
		-e 's/%%VERSION_LONG%%/$(VERSION_LONG)/;' \
		-e "s/%%LAST_UPDATE%%/$$htmldate/;" \
		-e "s/%%DATE%%/$$dbdate/;" \
		< $$tmpfile > $@; \
		chmod 644 $@

$(README_OUT):	$(README_IN) $(DB_SRC)
	@echo "making $@"; \
	dbdate="`./getdate -describe < $(DB_SRC)`"; \
	sed -e "s,%%DATE%%,$$dbdate,;" < $(README_IN) > $@

# Additional dependancies.

$(FINAL_DIR)/dbase.html::	$(HEAD) $(TAIL)
$(FINAL_DIR)/index.html::	$(HEAD) $(TAIL)
$(FINAL_DIR)/program.html::	$(HEAD) $(TAIL)
$(FINAL_DIR)/submit.html::	$(HEAD) $(TAIL)
$(FINAL_DIR)/view.html::	$(HEAD) $(TAIL)

