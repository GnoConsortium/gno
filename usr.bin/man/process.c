#ifdef __CCFRONT__
#include <14:pragma.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <gsos.h>
#include <orca.h>

#include "makewhatis.h"

/* These are the compression types */
#define NO_COMPRESS 0
#define  Z_COMPRESS 1
#define  F_COMPRESS 2

#ifdef TEST_PROCESS
	short v_flag    = 1;
	FILE *output_fp;
	FILE *error_fp;
#else
	extern FILE *output_fp;
	extern FILE *error_fp;
#endif

extern GSString255Ptr	__C2GSMALLOC (char *s);
extern char buffer[];
extern char titlebuf[];

/*
 * void process (char *filename, char *tmp_file, FILE *whatis_fp);
 *
 * Pre:	<filename> is the name of the file to add to the database.
 *			<tmp_file> is the name for the temporary file.  It has not been opened.
 *			<whatis_fp> is a file pointer to the open whatis database file.
 *
 * Post: The name, section number, and description from the man page is
 *			appended to <whatis_fp>.
 */

void process (char *filename, char *tmp_file, FILE *whatis_fp) {

	FileInfoRecGS info_record;		/* used to get the file type */
   char *suffix;                 /* points to the start of the file suffix */
   static char command_buf[255];	/* used to call system(2) */
	char *p1;							/* a scratch pointer */
   int namelength;					/* length of 'name (section)' */
   char *name;							/* points to the file basename */
   short compression;				/* the compression type (if nec) */

   if (v_flag>=2) fprintf(output_fp,"Working on file %s ...\n",filename);

	/*
    * get the file basename
    */

   if ((name = malloc (strlen(filename)+1)) == NULL) {
	   if (v_flag)
      	fprintf(error_fp,"malloc failed when processing %s -- file skipped\n",
	         filename);
      return;
   }
   strcpy(name,filename);

   /*
    * check for a compression suffix like ".Z" or ".F"
    */

   p1 = name + strlen(name) -1;			/* p1 points to last char of name */
   while ((p1>=name) && (*p1!='.')) --p1;
   if (p1<=name) { /* <= because we don't want a name beginning with '.' */
	   if (v_flag)
      	fprintf(error_fp,"%s has no suffix -- file skipped\n",
	         filename);
      return;
   }
   if ((strcmp(p1,".Z")==0) || (strcmp(p1,".z")==0)) {
	   compression = Z_COMPRESS;
   } else if ((strcmp(p1,".F")==0) || (strcmp(p1,".f")==0)) {
	   compression = F_COMPRESS;
   } else compression = NO_COMPRESS;
   *p1 = '\0';

   /*
    * define the suffix
    */

   if (compression == NO_COMPRESS) {
		suffix = p1 + 1;
   } else {
   	while ((p1>=name) && (*p1!='.')) --p1;
   	if (p1<=name) { /* <= because we don't want a name beginning with '.' */
	   	if (v_flag)
      		fprintf(error_fp,"%s has deformed file name -- file skipped\n",
	         	filename);
      	return;
   	}
      *p1 = '\0';
      suffix = p1 + 1;
   }

   /*
    * find out the file type
    */

   if (compression == NO_COMPRESS) {
   	info_record.pCount = 5;
   	info_record.pathname = __C2GSMALLOC(filename);
		GetFileInfoGS(&info_record);
   	if (toolerror()) {
	   	if (v_flag)
      		fprintf(error_fp,
            	"malloc failed when processing %s -- file skipped\n", filename);
      	return;
   	}
   }

	/*
    * Process the file according to type: nroff, aroff, freeze, and compress.
    * The digested result is placed in buffer.
    */

   if ((compression == NO_COMPRESS) &&
   	 (info_record.fileType == 0x50u) &&
       (info_record.auxType == 0x8010u)
      ) {
   	
      /* is it an Appleworks GS word processor document?  Use aroff */
      sprintf(command_buf,"aroff -b %s >%s",filename,tmp_file);
      system(command_buf);
		fillbuffer(tmp_file);

   } else if (compression == Z_COMPRESS) {
	
      /* Compressed man page; uncompress it */
      sprintf(command_buf,"compress -dc %s >%s",filename,tmp_file);
      system(command_buf);
		fillbuffer(tmp_file);

   } else if (compression == F_COMPRESS) {
	
      /* Frozen man page; melt it */
      sprintf(command_buf,"freeze -dc %s >%s",filename,tmp_file);
      system(command_buf);
		fillbuffer(tmp_file);

   } else if ((toupper(*suffix)=='L') && (*(suffix+1)=='\0')) {
	
  		/* It's a link to another man page; do nothing. */
		return;

   } else {

		/* Assume that it's a text file */
		fillbuffer(filename);
   }

   /*
    * If there was an error, or it is an .so reference to another man
    * page, then return without writing anything to the database
    */

   if ((buffer[0] == '\0') || (titlebuf[0] == '\0')) return;

   /*
    * At this point, buffer contains the line that we need to print to
    * whatis_fd.  Strip off any leading spaces, then give it a nice
    * formatting.
    */

  	p1 = buffer;
   while (isspace(*p1)) p1++; 
	namelength = strlen(titlebuf) + strlen(suffix) + 4;

  	if (namelength > (TABLENGTH * 3)) {
   	fprintf(whatis_fp,"%s (%s) - %s\n", titlebuf, suffix, p1);
   } else if (namelength > (TABLENGTH * 2)) {
     	fprintf(whatis_fp,"%s (%s)\t- %s\n", titlebuf, suffix, p1);
   } else if (namelength > TABLENGTH ) {
      fprintf(whatis_fp,"%s (%s)\t\t- %s\n", titlebuf, suffix, p1);
   } else {                               
      fprintf(whatis_fp,"%s (%s)\t\t\t- %s\n", titlebuf, suffix, p1);
   }

	return;
}


#ifdef TEST_PROCESS

int main (int argc, char **argv) {
	
   output_fp = stdout;
	error_fp  = stderr;

	if (argc != 2) {
	   fprintf(stderr,"Usage: %s <man_page_file_name>\n",argv[0]);
      return -1;
   }

	process (argv[1], ":tmp:garbage", stdout);

   return 0;
}

#endif
