/*
 * getvers version 2.0 for Apple IIGS
 *
 *  Written by Dave Tribby for GNO/ME 2.0.6  *  April 1998
 *  Original getvers written by Ian Schmidt (Copyright 1993 Two Meg Software)
 *
 *  NOTE: I started in recoding getvers in C, but shifted to assembly language
 *        and completed the implementation.  This version has a couple of
 *        deficiencies (e.g. the resources should be locked after being
 *        loaded). It's included just in case someone must translate getvers
 *        to C and needs a resonable starting point.
 *
 * $Id
 */


#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Types.h>
#include <Resources.h>
#include <GSOS.h>
#include <orca.h>
#include <misctool.h>

#include <gno/gno.h>

int rval=0;		/* Returned status */
int bflag=0;		/* Command line -b option flag */
int cflag=0;		/* Command line -c option flag */

static void usage __P((void));
static void ReadResources __P((char *));


int
main(int argc,
	char *argv[])
{
	int ch;
	char *fn;

	/* Report stack usage at end if compile option is set */
	__REPORT_STACK();

	/* Parse the command-line options */
	while ((ch = getopt(argc, argv, "bc")) != -1)
		switch (ch) {
		case 'c':
			cflag = 1;
			break;
		case 'b':
			bflag = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv = argv + optind;

	if (!*argv)
		usage();
	else   {

		/* Start up the resource manager */
		ResourceStartUp(userid());

		/* Parse the command-line filenames */
		while (*argv) {
			fn = *argv++;
			ReadResources(fn);
			if (*argv && !bflag) (void)printf("\n");
		}

		/* Shut down the resource manager     */
		/* (which closes open resource files) */
		ResourceShutDown();
	}
	exit(rval);
}

/*
 *   Perform malloc memory allocation + error checking
 */
static void *
malloc_c(size_t size)
{
	void *rtn_value;
	if ((rtn_value = malloc(size)) == NULL)   {
		(void)fprintf(stderr, "\nERROR: cannot allocate %ld bytes\n",
			size);
		exit(1);
	}
	return rtn_value;
}


typedef struct rVersType {
	Long		version;
	unsigned short	country;
	Str255		product;
} rVersType, *rVersPtr;

/*
 * Translate 8-bit text and print on stdout
 */
void
Print8bitText(ptr raw_text, Word raw_len)
{
	Word		Xlate_len;
	ResultBuf255Ptr	Xlate_buf;
	LongWord	Xresult;
	int		Xprint_len;

	/* Translated buffer could need up to 4 characters per input char. */
	/* It also needs two length words preceeding the characters.       */
	Xlate_len = raw_len*4+4;

	/* Allocate an extra byte so C-string can be terminated with null */
	Xlate_buf = (ResultBuf255Ptr)malloc_c(Xlate_len*4+1);
	Xlate_buf->bufSize = Xlate_len;

	Xresult = StringToText(fAllowLongerSubs, raw_text,
		raw_len, (Ptr)Xlate_buf);

	/* Add null character to terminate characters */
	Xprint_len = Xresult & 0xFFFF;
	Xlate_buf->bufString.text[Xprint_len] = 0;

	/* Print the translated text */
	(void)fputs(Xlate_buf->bufString.text, stdout);

	/* All done with the translation buffer */
	free(Xlate_buf);
}

static void
PrintPString(ptr ps)
{
	int len;
	len = *ps++;
	while (len--)   putchar(*ps++);
}

/*
 * Translate 8-bit characters that might be embedded in
 * the p-string parameter and print on stdout
 */
static void
Print8bitString(ptr ps)
{
	/* Length of input string */
	Print8bitText((ptr)(ps+1), *ps);
}



#define NUM_COUNTRY 55
char	*countryTbl[NUM_COUNTRY]={"United States", "France", "Britain",
		"Germany", "Italy", "Netherlands", "Belgium/Luxembourg",
		"Sweden", "Spain", "Denmark", "Portugal", "French Canadian",
		"Norway", "Israel", "Japan", "Australia", "Arabia", "Finland",
		"French Swiss", "German Swiss", "Greece", "Iceland", "Malta",
		"Cyprus", "Turkey", "Bosnia/Herzegovena/Yugoslavia/Croatia",
		NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		"India", "Pakistan",
		NULL, NULL, NULL, NULL, NULL, NULL,
		"Lithuania", "Poland", "Hungary", "Estonia", "Latvia",
		"Lapland", "Faeroe Islands", "Iran", "Russia", "Ireland",
		"Korea", "China", "Taiwan", "Thailand"};

/*
 * Print information related to the rVersion resource
 * passed as a parameter.
 */
static void
PrintVersionResource(ptr rezPtr)
{
	rVersPtr	rVers_addr;
	String(9)	string_version;
	unsigned long	moreInfo;
	int		country_num;
	char		*country;

	/* Translate version from raw number to string */
	rVers_addr = (rVersPtr)rezPtr;
	VersionString(0, rVers_addr->version, (Ptr)&string_version);

	/* Print product name and string version */
	Print8bitString((ptr) &(rVers_addr->product));
	putchar(' ');
	PrintPString((ptr) &string_version);
	putchar('\n');

        /* All done if -b (brief) option was specified */
	if ( bflag ) return;

	/* The "moreInfo" p-string follows the product p-string */
	moreInfo = (unsigned long)&(rVers_addr->product) +
			rVers_addr->product.textLength + 1;
	Print8bitString((Ptr)moreInfo);
	putchar('\n');

	/* Print the country information */
	if ( (country_num=rVers_addr->country) < NUM_COUNTRY)
		country = countryTbl[country_num];
	if (country_num >= NUM_COUNTRY || country == NULL )
		(void)printf("Country: %d unknown to this version of getvers\n",
			country_num);
	else
		(void)printf("Country: %s\n", country);
}


/*
 * Print the rVersion comment passed as a parameter.
 */
static void
PrintCommentResource(ptr rezPtr)
{
	LongWord	rez_len;
	rez_len = GetResourceSize(0x802A,1);
	Print8bitText(rezPtr, rez_len);
	putchar('\n');
}


/*
 * Read the version (and, optionally, comment) resources from the
 * indicated file
 */
static void
ReadResources(char *path_str)
{
	#define noPreload 0x8000
	Word		new_file_id;
	Word		old_file_id;
	Word		old_depth;
	GSString255Ptr	path_ptr;
	Handle		rVer_handle, rCom_handle;
	int		error;

	/* Let user know what file we're working on */
	(void)printf("%s: ",path_str);

	/* Convert C string into GSString */
	path_ptr = malloc_c(strlen(path_str)+2);
	path_ptr->length = strlen(path_str);
	strcpy(path_ptr->text, path_str);

	/* Open the resource fork of the file */
	new_file_id = OpenResourceFile(noPreload+readEnable,
			NULL, (Pointer)path_ptr);
	if ((error = toolerror()) != noError) {
		if (error == 0x0046)
			(void)printf("File not found\n");
		else if (error == 0x0063)
			(void)printf("No resource fork\n");
		else
			(void)printf("Error $%04X opening resource fork\n",
					error);
		rval = 1;
	}
	else {
		/* Set up to search only this resource file, remembering    */
		/* previous setting so it can be restored when closing file */
		old_file_id = GetCurResourceFile();
		SetCurResourceFile(new_file_id);
		old_depth = SetResourceFileDepth(1);

		/* Read the version resource */
		rVer_handle = LoadResource(0x8029,1);
		if ((error = toolerror()) != noError) {
			if (error == 0x1E06)
				(void)printf("No version resource\n");
			else
				(void)printf(
				  "Error $%04X getting version resource\n",
					error);
			rval = 1;
		}
		else {
			PrintVersionResource(*rVer_handle);
		}

		/* Read the comment resource, if requested */
		if ( cflag )   {
			rCom_handle = LoadResource(0x802A,1);
			if ((error = toolerror()) == noError) {
				PrintCommentResource(*rCom_handle);
			}
		}

		/* Close the resource file and restore original search values */
		CloseResourceFile(new_file_id);
		SetCurResourceFile(old_file_id);
		SetResourceFileDepth(old_depth);
	}
	/* Free the memory allocated for the path name */
	free(path_ptr);
}

static void
usage(void)
{
	(void)fprintf(stderr, "usage: getvers [-b] [-c] file ...\n");
	exit(1);
}
