# Create the test environment for tr

# Compile and load flags passed to occ
#   -v: verbose
#   -I: add directory to header path
CFLAGS	+= -v -I /usr/include
LDFLAGS	+= -v

GENDATA	= file2.bin1 file2.bin2 file2.bin3 \
	  file3.alnum file3.alpha file3.blank file3.cntrl file3.digit \
	  file3.full file3.graph file3.lower file3.print file3.punct \
	  file3.space file3.upper file3.xdigit 

# Default target, "all," generates the files
all: file2.bin1 genbin

gendata: $(GENDATA)

$(GENDATA) .UPDATEALL: genbin
	./genbin

# Remove intermediate files
clean:
	-$(RM) -f genbin.o genbin.root

clobber: clean
	-$(RM) -f test.list $(GENDATA) genbin
