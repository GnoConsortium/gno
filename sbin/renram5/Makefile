CFLAGS	= -w -O
LDFLAGS	=

renram5: renram5.o renram5.r
	$(CC) $(LDFLAGS) renram5.o $(LDLIBS) -o $@
	copyfork renram5.r renram5 -r

clean:
	$(RM) -f renram5.o renram5.root renram5.r

clobber: clean
	$(RM) -f renram5
