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

all:  h ps

h:	gsh.sgml $(CHAPTERS)
	sgmltools -b html gsh.sgml
	../posthtml gsh/*

ps:	gsh.sgml $(CHAPTERS)
	sgmltools -b ps gsh.sgml

gsh.sgml:	main.sgml
	date="`../getdate -date main.sgml $(CHAPTERS)`"; \
	echo "generating $@"; \
	perl -pe "s,%%%date%%%,$$date,g;" < $< > $@

.PRECIOUS: gsh.sgml

clean:
	rm -f gsh.sgml *~

clobber: clean
	rm -rf gsh
