/*
 * Copyright 1995-1998 by Devin Reade <gdr@trenco.gno.org>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 *
 * $Id: fillbuffer.c,v 1.4 1998/03/29 07:15:55 gdr-ftp Exp $
 */

#ifdef __ORCAC__
segment "makewhatis";
#pragma noroot
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <err.h>
#include "man.h"

#define NAME1 "NAME"
#define NAME2 "N\bNA\bAM\bME\bE"
#define NAME3 "N\bN\bN\bNA\bA\bA\bAM\bM\bM\bME\bE\bE\bE"
#define SYNOPSIS1 "SYNOPSIS"      
#define SYNOPSIS2 "S\bSY\bYN\bNO\bOP\bPS\bSI\bIS\bS"      
#define SYNOPSIS3 "S\bS\bS\bSY\bY\bY\bYN\bN\bN\bNO\bO\bO\bOP\bP\bP\bPS\bS\bS\bSI\bI\bI\bIS\bS\bS\bS"      
#define DESCRIPTION1 "DESCRIPTION"
#define DESCRIPTION2 "D\bDE\bES\bSC\bCR\bRI\bIP\bPT\bTI\bIO\bON\bN"
#define DESCRIPTION3 "D\bD\bD\bDE\bE\bE\bES\bS\bS\bSC\bC\bC\bCR\bR\bR\bRI\bI\bI\bIP\bP\bP\bPT\bT\bT\bTI\bI\bI\bIO\bO\bO\bON\bN\bN\bN"

       char buffer[BUFFERSIZE];     /* contains the command description  */
       char titlebuf[BUFFERSIZE];   /* contains the command name         */
static char buffer2[BUFFERSIZE];    /* used for chars read from man page */

#ifdef TEST_FILLBUFFER
   short v_flag=2;
#  define output_fp stdout
#  define error_fp  stderr
#else
   extern FILE *output_fp;       /* output file descriptor -- may be stdout */
   extern FILE *error_fp;        /* error file descriptor  -- may be stderr */
#endif

/* void fillbuffer (char *filename);
 *
 * Pre:  <filename> is the name of the man page or temporary file
 *       containing text.  It may have nroff formatting information
 *       or screen control codes, but must not be either a compressed
 *       file nor an AWGS word processor file.
 *
 * Post: <buffer> will contain all the text, minus formatting and control
 *       code, starting from the word "NAME" and ending with either ".SH".
 *       "SYNOPSIS", or "DESCRIPTION", whichever comes first.  <titlebuf>
 *       will contain all printable text starting with the first printable
 *       non-whitespace character following "NAME" and ending with the
 *       last printable character before the first '-' following "NAME".
 *
 *       If an error occurs, <buffer> will be an empty string (ie:
 *       buffer[0] == '\0')
 *
 * Warning:  This routine was written to be fast at the expense of code
 *           size.  It also has a lot of "special case"ing since it could
 *           be fed nroff source, aroff'd output, or text files that may
 *           include formatting control codes.  If you're looking for some
 *           nice neat code, you're not going to find it here.
 */

void fillbuffer (char *filename) {
   FILE *fp;      /* FILE pointer for filename                    */
   int count;     /* how many chars were read into buffer2        */
   char *p1;      /* points to current char in buffer2            */
   char *p2;      /* points to last char (of interest) in buffer2 */
   char *p3;      /* points to current char in buffer             */
   char *p6;      /* scratch */
   short found;   /* some flags */
   short in_comment;
   short in_format_BR;
   short in_format_f;
   short foo;

   /*
    * Set p4 and p5 to the ends of buffer and titlebuf, respectively.
    * These are used for error checking, so that we don't overflow the
    * buffers.  Using pointers will speed things up a bit at the cost
    * of four bytes of local storage.  They are not global for the sake
    * of speed.
    */

   char *p4 = buffer   + BUFFERSIZE;
   char *p5 = titlebuf + BUFFERSIZE;

   /*
    * open the file
    */

   if ((fp = fopen(filename,"r")) == NULL) {
      buffer[0] = '\0';
      if (v_flag) fprintf (error_fp,"Open failed for file \"%s\"\n",filename);
      return;
   }

   /*
    * see if it includes another man page
    */
   if ((fgets(buffer2,4,fp) == NULL) || (strncmp(buffer2,".so",3)==0)) {
      buffer[0] = '\0';
      titlebuf[0] = '\0';
      fclose(fp);
      return;
   }
   fseek(fp,0L,SEEK_SET);

   /*
    * Make fp point to the first newline following NAME so that the
    * next block read will pick it up as the first character.  This is
    * needed for the next section of code following this one.
    */

   for(;;) {

      /*
       * read in buffer2 in a line-oriented fashion at first so that we
       * can more easily ignore .\" and .TH lines
       */

      if (fgets(buffer2,BUFFERSIZE,fp)==NULL) {
         /*
          * eof or error, and we haven't found "NAME" yet ... return
          * an empty string
          */
          buffer[0] = '\0';
          titlebuf[0] = '\0';
          fclose(fp);
          if (v_flag) fprintf (error_fp,
               "EOF or error on %s, NAME not found.\n",filename);
          return;
      }

      /* ignore comment lines and any .TH line(s) */
      if ((strncmp(buffer2,".\\\"",3)==0) || (strncmp(buffer2,".TH",3)==0))
         continue;

      /* check the various versions of "NAME" */
      if (strstr(buffer2,NAME1) != NULL) break;
      if (strstr(buffer2,NAME2) != NULL) break;
      if (strstr(buffer2,NAME3) != NULL) break;
   }                             


   /* we need the previous newline for the next algorithm to work */
   ungetc('\n', fp);

   /*
    * Make p1 point to spot in buffer2 where there occurs the first
    * character following '-' which in turn follows "NAME".
    * Note that if "NAME" or '-' are within a comment line
    * (nroff source), it will still be picked up.
    *
    * Also copy selected chars to titlebuf until the first '-' is found.
    */

   p3 = titlebuf;
   found = 0;        /* set this when we find '-'        */
   in_format_BR = 0; /* in the middle of a .BR format    */
   in_format_f = 0;  /* in the middle of a \fI format    */
   in_comment = 0;   /* in the middle of a .\" comment   */
   foo = 0;          /* haven't found the printable character after NAME */
   for(;;) {                             

      /* read another block into buffer2. */    

      count = fread(buffer2,sizeof(char),BUFFERSIZE-1,fp);
      if (count == 0) {
         /* eof or error; empty buffer and titlebuf then return */
         buffer[0] = '\0';
         titlebuf[0] = '\0';
         fclose(fp);
         if (v_flag) fprintf (error_fp,
               "EOF or error on %s, command name not found.\n",filename);
         return;
      }
      buffer2[count] = '\0';
      p1 = buffer2;

      /* mark the "end" of buffer2 with p2 */
      if ((p2 = strchr(p1,'-')) != NULL) {
         found = 1;
      } else {
         p2 = buffer + count;
      }

      /*
       * If this is not our first iteration, dump any formatting information
       * or comments left over from the last interation.
       */

      if (in_comment) {
         while((p1<p2) && (*p1 != '\n')) p1++;
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
       * At this time, p1 points to the start of the key words, p2
       * to either the end of the key words or the end of buffer2,
       * and p3 into the spot in titlebuf where the next character should
       * be put.
       *
       * Copy *p1 to *p3 while p1<p2 (not at end of titlebuf or key words),
       * skipping comments, formatting info, and control chars
       */

      for (; p1<p2; p1++) {

         /* skip .\" comments */
         if (strncmp(p1,"\n.\\\"",4) == 0) {
            while ((p1<p2) && (*p1!='\n')) p1++;
            if (p1==p2) in_comment = 1;
            continue;
         }

         /* skip .BR-type formatting */
         if ((p1<p2) && (*p1=='\n') && (*(p1+1)=='.')) {
            p1++;
            while ((p1<p2) && !isspace(*p1)) p1++;
            if (p1==p2) in_format_BR = 1;
            else --p1;
            continue;
         }

         /* skip \fI-type formatting */
         if ((p1<p2) && (*p1=='\\') && (*(p1+1)=='f')) {
            if ((p1 + 3) < p2) {
               p1 += 3;
            } else {
               in_format_f = p2 - p1;
               p1 = p2;
            }
            continue;
         }

         /*
          * skip whitespace if we haven't got the beginning of the
          * description yet.
          */

#ifdef ISGRAPH_FIX
         if (isgraph(*p1) && (*p1!=' ')) foo=1;
         if (!foo) {
            while ((p1<p2) && !(isgraph(*p1) && (*p1!=' '))) p1++;
            if ((*p1=='.') && (*(p1-1)=='\n')) p1 -=2;
            else --p1;
            continue;
         }
#else 
         if (isgraph(*p1)) foo=1;
         if (!foo) {
            while ((p1<p2) && !isgraph(*p1)) p1++;
            if ((*p1=='.') && (*(p1-1)=='\n')) p1 -=2;
            else --p1;
            continue;
         }
#endif

         /*
          * at this point, *p1 is either a control char or something that
          * we want in titlebuf, assuming that p1<p2.
          */

         if ((p1<p2) && !iscntrl(*p1)) {

            /*
             * The conditional below means:
             * Copy it so that:  1. There is only one space between words; and
             *                   2. The buffer doesn't begin with a space.
             */

            if (  !((p3>titlebuf) && (*p1 == ' ') && (*(p3-1) == ' ')) &&
                  !((p3==titlebuf) && (*p3 == ' '))
               ) {

               /* don't let a space precede a comma */
               if ((*p1==',') && (*(p3-1)==' ')) {
                  *(p3-1) = ',';
                  continue;
               } else *p3++ = *p1;
               if (p3>=p5) {  /* titlebuf overflow? */
                  if (v_flag)
                     fprintf(error_fp,"command name buffer overflow on %s\n",
                             filename);
                  buffer[0] = '\0';
                  titlebuf[0] = '\0';
                  fclose(fp);
                  return;
               }
            }
         }
      }
   
      if (found) {      /* we've got all of the key words */
         p3--;          /* p3 now points to last char, not terminator */
         if (*p3=='\\') p3--;
         while(isspace(*p3)) p3--;
         *(p3+1) = '\0';
         break;   
      }

   }
   p1 = p2 + 1;
#ifdef ISGRAPH_FIX
   while ( (p1 < buffer2 + BUFFERSIZE) &&!(isgraph(*p1) && (*p1 != ' '))) p1++;
#else 
   while ((p1 < buffer2 + BUFFERSIZE) && !isgraph(*p1)) p1++;
#endif

   /*
    * now copy selected chars to buffer until the next subheading is found
    */

   p3 = buffer;
   found = 0;        /* set this when we find one of the above strings  */
   in_format_BR = 0; /* in the middle of a .BR format                   */
   in_format_f = 0;  /* in the middle of a \fI format                   */
   in_comment = 0;   /* in the middle of a .\" comment                  */
   for(;;) {

      /* mark the "end" of buffer2 with p2 */
      if ( ((p2 = strstr(p1,".SH"))         != NULL) ||
           ((p2 = strstr(p1,SYNOPSIS1))     != NULL) ||
           ((p2 = strstr(p1,SYNOPSIS2))     != NULL) ||
           ((p2 = strstr(p1,SYNOPSIS3))     != NULL) ||
           ((p2 = strstr(p1,DESCRIPTION1))  != NULL) ||
           ((p2 = strstr(p1,DESCRIPTION2))  != NULL) ||
           ((p2 = strstr(p1,DESCRIPTION3))  != NULL)
         ) {
         *p2 = '\0';
         /*
          * this conditional is to cover the wierd case of having the word
          * "SYNOPSIS" appearing in the description (or elsewhere), as
          * it does for the GNO Intro(1) man page.  Blech.  Only in
          * aroff source or a preformatted page would this matter.
          */
         if (((p6 = strstr(p1,SYNOPSIS1))     != NULL) ||
             ((p6 = strstr(p1,DESCRIPTION1))  != NULL)) {
            p2 = p6;
         }
         found = 1;
      } else {
         p2 = buffer + count;
      }

      /*
       * If this is not our first iteration, dump any formatting information
       * or comments left over from the last interation.
       */

      if (in_comment) {
         while((p1<p2) && (*p1 != '\n')) p1++;
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
         if (strncmp(p1,"\n.\\\"",4) == 0) {
            while ((p1<p2) && (*p1!='\n')) p1++;
            if (p1==p2) in_comment = 1;
         }
      
         /* skip .BR-type formatting */
         if ((p1<p2) && (*p1=='\n') && (*(p1+1)=='.')) {
            p1++;
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

         if ((p1<p2) && !iscntrl(*p1)) {

            /*
             * The conditional below means:
             * Copy it so that:  1. There is only one space between words; and
             *                   2. The buffer doesn't begin with a space.
             */

            if (  !((p3>buffer) && (*p1 == ' ') && (*(p3-1) == ' ')) &&
                  !((p3==buffer) && (*p3 == ' '))
               ) {
               *p3++ = *p1;
               if (p3>=p4) {  /* buffer overflow? */
                  if (v_flag)
                     fprintf(error_fp,"command description buffer overflow on %s\n",
                             filename);
                  buffer[0] = '\0';
                  titlebuf[0] = '\0';
                  fclose(fp);
                  return;
               }
            }
         }   
      }
   
      if (found) {      /* we've got the entire description */
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
         if (v_flag) {
#define NO_DESCRIPTION "description not found"
           if (feof(fp)) {
             warnx("EOF on %s, %s", filename, NO_DESCRIPTION);
           } else if (ferror(fp)) {
             warn("error on %s, %s", filename, NO_DESCRIPTION);
           } else {
             errx(1, "fread above line %d returned zero", __LINE__);
           }
         }
         fclose(fp);
         return;
      }
      buffer2[count] = '\0';
      p1 = buffer2;
      
   }

   /*
    * close the file
    */

    fclose(fp);

    return;
}
