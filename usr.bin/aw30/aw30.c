#pragma keep "AppleWorks"

#pragma stacksize 2048

#pragma optimize -1

#include <stdio.h>
#include <stdlib.h>

#pragma lint -1


#define PROG_VERSION "0.70"
#define DATE "11/21/92"

FILE *f;
char SFMinVers;		/* Minimum AppleWorks Version required */
int MultRulers;		/* Boolean - Multiple Rulers */
char Ruler[80];		/* Ruler for Tab Stops */
int *input;
int lcv;
int *ID1, *ID2;
char Center;		/* Boolean - Centering on */
char Codes;		/* Boolean - Special Codes switch */
char NewPage;		/* Boolean - Use New Page */
char LeftMargin;	/* Size of left margin (tenths of an inch) */
char RightMargin;	/* Size of right margin (tenths of an inch) */
char PlatenWidth;	/* Width of platen (tenths of an inch) */
char argument;
char ShowNames;		/* Boolean - Show names of file being processed */

void usage (void)

{
fprintf (stderr, " Version %s (%s)\n", PROG_VERSION, DATE);
fprintf (stderr, " Robert Hill\n");
fprintf (stderr, " INTERNET: rhill@oread.cc.ukans.edu\n AOL: RobertHill\n\n");
fprintf (stderr, "Usage: appleworks [-cn] filename\n");
}


int initialize (void)

{
fseek (f, 4L, SEEK_SET);
*input = (char) fgetc (f);
                
if (*input != 79) {
	fprintf (stderr, "appleworks: File is not an AppleWorks file.\n");
	return -1;
}

for (lcv = 0; lcv < 80; ++lcv) {
        Ruler [lcv] = (char) fgetc (f);
        if (Ruler[lcv] == EOF) {
	        fprintf (stderr, "appleworks: File is corrupted.\n");
		return -1;
        }
}

fseek (f, 177L, SEEK_SET);
MultRulers = (char) fgetc (f);
fseek (f, 184L, SEEK_SET);
SFMinVers = (char) fgetc (f);
fseek (f, 300L, SEEK_SET);

return 0;
}


int TextLine (void)

{
char indent;
int length;
float location;

*input = (char) fgetc (f);

if (*input == EOF)
	return -1;
indent = (*input & 127);
for (lcv = 0; lcv < indent; ++ lcv)
	printf (" ");

*input = (char) fgetc (f);

if (*input == EOF)
	return -1;
length = (*input) & 127;

if (Center) {
	location = (((PlatenWidth - RightMargin - LeftMargin) / 10.0) -
        		length) / 2.0;
	for (lcv = 0; lcv < (int) location; ++ lcv)
	        printf (" ");
}

for (lcv = 0; lcv < length; ++ lcv) {
        *input = (char) fgetc (f);
        if (*input == EOF)
	        return -1;
        else if (((*input) > 0x18) && ((*input) <= 0x7F))
		printf ("%c", *input);
	else switch (*input) {
                case 0x01 : 	printf ("%c", 15);	/* Boldface on */
				break;
                case 0x02 : 	printf ("%c", 14);	/* Boldface off */
				break;
                case 0x07 : 	printf ("%c", 15);	/* Underline on */
				break;
                case 0x08 : 	printf ("%c", 14);	/* Underline off */
				break;
	        case 0x16 : 	printf (" ");
				++indent;
				while ((Ruler[indent]=='=') && (indent < 81)) {
                                	printf (" ");	/* Tab */
					++indent;
				}
				break;
	}
                
}
printf("\n");
}



void HandleCommand (int *ID1, int *ID2)

{
switch (*ID2) {
	case 0xD8	:	PlatenWidth = *ID1;
				if (Codes)
                                	printf ("---Platen Width=%.1f inches\n",
                                        	(float) (PlatenWidth / 10));
				break;
	case 0xD9	:	LeftMargin = *ID1;
				if (Codes)
                                	printf ("---Left Margin=%.1f inches\n",
                                        	(float) (LeftMargin / 10));
				break;
	case 0xDA	:	RightMargin = *ID1;
				if (Codes)
                                	printf ("---Right Margin=%.1f inches\n",
                                        	(float) (RightMargin / 10));
				break;
        case 0xE0	:	Center = 0;
				if (Codes)
                                	printf ("---Unjustified\n");
			        break;
	case 0xE1	:	Center = 1;
				if (Codes)
					printf ("---Center\n");
			        break;
	case 0xE9	:	if (NewPage)
					printf ("%c", 12);
			        break;
}
}



int main (int argc, char **argv)
{
argument = 1;
Codes = NewPage = ShowNames = 0;

input = (int *) malloc (sizeof(int));
ID1 = (int *) malloc (sizeof(int));
ID2 = (int *) malloc (sizeof(int));                                           

if (argc == 1)
	usage ();
else {
    while (argc > argument) {
	Center = 0;
	LeftMargin = RightMargin = 10;
	PlatenWidth = 80;
	if ((argument == 1) && (argv[1][0] == '-')) {
                lcv = 1;
                while (argv[1][lcv] != 0x00) {
			switch (argv[1][lcv]) {
	                        case 'c' : 	Codes = 1;
						break;
                                case 'n' :	NewPage = 1;
						break;
                                case 's' :	ShowNames = 1;
						break;
			}
			++lcv;
                }
	        if (argc > 2) {
	                ++argument;
	                f = fopen (argv[2], "rb");
			if (f == NULL) {
				fprintf (stderr,
					"appleworks: Could not open %s.\n",
					argv[2]);
				return -1;
			}
			else if (ShowNames)
				printf ("appleworks: %s\n\n", argv[2]);
		}
		else {
			usage ();
			return 0;
		}
	                
        }
	else {
		f = fopen (argv[argument], "rb");
		if (f == NULL) {
			fprintf (stderr, "appleworks: Could not open %s.\n",
				argv[argument]);
			return -1;
		}
		else if (ShowNames)
			printf ("appleworks: %s\n\n", argv[argument]);
	}

	if (initialize () == -1) 
		return -1;

	while ((!((*ID1==0xFF) && (*ID2==0xFF))) &&
	      		(!((*ID1==EOF) || (*ID2==EOF)))) {

		*ID1 = fgetc (f);
		*ID2 = fgetc (f);

		if ((*ID1 != EOF) && (*ID2 != EOF)) {
	                if (*ID2==0x00)
	                        if (TextLine () == -1) {
                                	fprintf (stderr,
                                          "appleworks: %s is corrupted.\n",
                                          	argv[argument]);
					return -1;
				}
			if (*ID2==0xD0)
	                        printf ("\n");
			if ((*ID2) > 0xD0)
	                        HandleCommand (ID1, ID2);
		}
	}
        *ID1 = *ID2 = 0;
        ++argument;
        printf ("\n");
        fclose (f);
    }
}

return 0;
}
