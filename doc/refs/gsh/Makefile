CHAPTERS = \
	start.sgml \
	interact.sgml \
	product.sgml \
	commands.sgml \
	vars.sgml \
	prefix.sgml \
	errors.sgml \
	comply.sgml \
	termcap.sgml \
	glossary.sgml

all:	h ps
h:	gsh/index.html
ps:	gsh.ltr.ps.Z

gsh.ltr.ps.Z: gsh.ltr.ps
	compress < $< > $@

gsh/index.html: gsh.sgml $(CHAPTERS)
	sgmltools -b html gsh.sgml
	../posthtml gsh/*

gsh.ltr.ps:	gsh.sgml $(CHAPTERS)
	sgmltools -b ps gsh.sgml
	mv gsh.ps gsh.ltr.ps

bork:
	true; echo "result is $$?"; exit 0

#	echo "$(SHELLMETAS)"

gsh.sgml: main.sgml $(CHAPTERS)
	mydate=`../getdate -date main.sgml $(CHAPTERS)`; \
	echo "generating $@"; \
	true perl -pe "s,%%%date%%%,$$mydate,g;"

# < main.sgml

# > $@

clean:
	rm -f gsh.sgml *~

clobber: clean
	rm -rf gsh gsh.ltr.ps gsh.ltr.ps.Z
