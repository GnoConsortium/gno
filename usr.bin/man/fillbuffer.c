#ifdef __CCFRONT__
#include <14:pragma.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "makewhatis.h"

		 char buffer[BUFFERSIZE];
static char buffer2[BUFFERSIZE];

/* void fillbuffer (char *filename);
 *
 * Pre:  <filename> is the name of the man page or temporary file
 *       containing text.  It may have nroff formatting information
 *       or screen control codes, but must not be either a compressed
 *       file nor an AWGS word processor file.
 *
 * Post: <buffer> will contain all the text, minus formatting and control
 *       code, starting from the word "NAME" and ending with either ".SH".
 *       "SYNOPSIS", or "DESCRIPTION", whichever comes first.
 *
 *       If an error occurs, <buffer> will be an empty string (ie:
 *       buffer[0] == '\0')
 */

void fillbuffer (char *filename) {
	FILE *fp;      /* FILE pointer for filename                    */
   int count;		/* how many chars were read into buffer2        */
   char *p1;	   /* points to current char in buffer2            */
   char *p2;      /* points to last char (of interest) in buffer2 */
   char *p3;		/* points to current char in buffer             */
   short found;	/* some flags */
	short in_comment;
   short in_format_BR;
   short in_format_f;

	/*
    * open the file
    */

   if ((fp = fopen(filename,"rb")) == NULL) {
	   buffer[0] = '\0';
      return;
   }

   /*
    * Make p1 point to spot in buffer2 where the first "NAME" occurs.
    * Note that if "NAME" is within a comment line (nroff source), it
    * will still be picked up.
    */

   for(;;) {
	   count = fread(buffer2,sizeof(char),BUFFERSIZE-1,fp);
      if (count == 0) {
         /*
          * eof or error, and we haven't found "NAME" yet ... return
          * an empty string
          */
          buffer[0] = '\0';
          fclose(fp);
          return;
      }
   	buffer2[count] = '\0';
      p1 = strstr(buffer2,"NAME");
      if (p1 != NULL) break;
   }

   /*
    * Make p1 point to spot in buffer2 where there occurs the first
    * character following '-' which in turn follows "NAME".
    * Note that if "NAME" or '-' are within a comment line
    * (nroff source), it will still be picked up.
    */

   for(;;) {
   	p2 = strchr(p1,'-');
   	if (p2 != NULL) {
	      p2++;
      	break;
      }
	   count = fread(buffer2,sizeof(char),BUFFERSIZE-1,fp);
      if (count == 0) {
         /*
          * eof or error, and we haven't found '-' yet ... return
          * an empty string
          */
          buffer[0] = '\0';
          fclose(fp);
          return;
      }
   	buffer2[count] = '\0';
      p1 = buffer2;
   }
   p1 = p2;
	
   /*
    * now copy selected chars to buffer until the next subheading is found
    */

   p3 = buffer;
	found = 0;			/* set this when we find one of the above strings 	*/
   in_format_BR = 0;	/* in the middle of a .BR format 						*/
   in_format_f = 0;	/* in the middle of a \fI format 						*/
   in_comment = 0;	/* in the middle of a .\" comment 						*/
   for(;;) {

      /* mark the "end" of buffer2 with p2 */
      if ( ((p2 = strstr(p1,".SH"))         != NULL) ||
	        ((p2 = strstr(p1,"SYNOPSIS"))    != NULL) ||
           ((p2 = strstr(p1,"DESCRIPTION")) != NULL)
         ) {
	      found = 1;
      } else {
	      p2 = buffer + count;
      }

      /*
       * If this is not our first iteration, dump any formatting information
       * or comments left over from the last interation.
       */

      if (in_comment) {
	      while((p1<p2) && (*p1 != '\r')) p1++;
         in_comment = 0;
      }

      if (in_format_BR) {
	      while ((p1<p2) && !isspace(*p1)) p1++;
         in_format_BR = 0;
      }
      
      if (in_format_f) {
	      p1 = p1 + 3 - in_format_f;
         in_format_f = 0;
      }

      /*
       * At this time, p1 points to the start of the description, p2
       * to either the end of the description or the end of buffer2,
       * and p3 into the spot in buffer where the next character should
       * be put.
       *
       * Copy *p1 to *p3 while p1<p2 (not at end of buffer or description),
       * skipping comments, formatting info, and control chars
       */

      for (; p1<p2; p1++) {

			/* skip .\" comments */
         if (strncmp(p1,"\r.\\\"",4) == 0) {
				while ((p1<p2) && (*p1!='\r')) p1++;
            if (p1==p2) in_comment = 1;
         }
      
         /* skip .BR-type formatting */
			if ((p1<p2) && (*p1=='\r') && (*(p1+1)=='.')) {
				while ((p1<p2) && !isspace(*p1)) p1++;
            if (p1==p2) in_format_BR = 1;
         }

         /* skip \fI-type formatting */
			if ((p1<p2) && (*p1=='\\') && (*(p1+1)=='f')) {
	         if ((p1 + 3) < p2) {
	            p1 += 3;
            } else {
	         	in_format_f = p2 - p1;
	            p1 = p2;
            }
         }

         /*
          * at this point, *p1 is either a control char or something that
          * we want in buffer, assuming that p1<p2.
          */

         if ((p1<p2) && !iscntrl(*p1)) *p3++ = *p1;
      }
	
      if (found) {		/* we've got the entire description */
	      *p3 = '\0';
      	break;	
      }

      /*
       * We're part way through the description; read another block
       * into buffer2.
       */ 	

	   count = fread(buffer2,sizeof(char),BUFFERSIZE-1,fp);
      if (count == 0) {
         /* eof or error; terminate buffer and return */
          *p3 = '\0';
          fclose(fp);
          return;
      }
	   p1 = buffer2;
	   
	}
  
   /*
    * close the file
    */

    fclose(fp);

    return;
}


#ifdef TEST_FILLBUFFER

int main(int argc, char **argv) {
	
	if (argc != 2) {
	   printf("Usage: %s <man_page_file_name>\n\n",argv[0]);
      return -1;
   }

   fillbuffer(argv[1]);

   if (strlen(buffer) == 0) {
	   printf("buffer empty\n");
   } else {
   	printf("main: buffer is %u chars long\n%s\n",strlen(buffer),buffer);
   }

   return 0;
}

#endif
