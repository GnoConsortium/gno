/*
 * Generate binary files for testing tr
 *
 */

#include <ctype.h>
#include <stdio.h>

static int ch;
static FILE *bf;

/* Generate a file of the given name to include bytes between low and high */
static void
genfile(char *name, int low, int high, int flag)
{

        (void) printf("Creating file %s\n", name);
        if ( (bf = fopen(name, "wb")) == NULL )   {
	        perror("Opening file");
                return;
                }
	for (ch=low; ch<=high; ch++)   {
	        switch (flag)   {
			case 1:	if ( !isalnum(ch) ) continue;
	                        break;
			case 2:	if ( !isalpha(ch) ) continue;
	                        break;
			case 3:	if ( !iscntrl(ch) ) continue;
	                        break;
			case 4:	if ( !isdigit(ch) ) continue;
	                        break;
			case 5:	if ( !isgraph(ch) ) continue;
	                        break;
			case 6:	if ( !islower(ch) ) continue;
	                        break;
			case 7:	if ( !isprint(ch) ) continue;
	                        break;
			case 8:	if ( !ispunct(ch) ) continue;
	                        break;
			case 9:	if ( !isspace(ch) ) continue;
	                        break;
			case 10: if ( !isupper(ch) ) continue;
	                        break;
			case 11: if ( !isxdigit(ch) ) continue;
	                        break;
			case 12: if ( !isblank(ch) ) continue;
			}
		(void) putc(ch, bf);
                }
        /* Final character is a blank when flagged */
        if ( flag == -1 )
	        (void) putc(' ', bf);
        (void) fclose(bf);
}

int
main(int argc, char *argv[])
{

/* Starting binary file: file2.bin1 */
genfile("file2.bin1", 0, 32, 0);

/* Binary file after translation: file2.bin2 */
genfile("file2.bin2", 128, 159, -1);

/* Binary file after translation of special characters */
(void) printf("Creating file file2.bin3\n");
if ( (bf = fopen("file2.bin3", "wb")) == NULL )   {
	perror("Opening file file2.bin3");
	}
else   {
	for (ch=0; ch<=32; ch++)
                switch (ch)   {
	        	case '\a':
				(void) putc('A', bf);
                                break;
	        	case '\b':
				(void) putc('B', bf);
                                break;
	        	case '\t':
				(void) putc('T', bf);
                                break;
	        	case '\n':
				(void) putc('N', bf);
                                break;
	        	case '\v':
				(void) putc('V', bf);
                                break;
	        	case '\f':
				(void) putc('F', bf);
                                break;
	        	case '\r':
				(void) putc('R', bf);
                                break;
	        	default:
				(void) putc(ch, bf);
                        }
        (void) fclose(bf);
        }

/* Full ASCII set: file3.full */
genfile("file3.full", 0, 127, 0);

/* Generate subsets that meet class criteria */
genfile("file3.alnum", 0, 127, 1);
genfile("file3.alpha", 0, 127, 2);
genfile("file3.cntrl", 0, 127, 3);
genfile("file3.digit", 0, 127, 4);
genfile("file3.graph", 0, 127, 5);
genfile("file3.lower", 0, 127, 6);
genfile("file3.print", 0, 127, 7);
genfile("file3.punct", 0, 127, 8);
genfile("file3.space", 0, 127, 9);
genfile("file3.upper", 0, 127, 10);
genfile("file3.xdigit", 0, 127, 11);
genfile("file3.blank", 0, 127, 12);

return 0;
}
