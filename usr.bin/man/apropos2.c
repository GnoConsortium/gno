/*
 * Copyright 1995 by Devin Reade <gdr@myrias.com>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 */

segment "apropos2__";

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include "util.h"
#include "man.h"

/*
 * orcapadding -- return a suitable number of padding tabs based on the
 *                length of the command
 */

static char *orcapadding (char *command) {
   int i;
   static char *two   ="\t\t";
   static char *one   ="\t";
   static char *zero  =" ";

   i = strlen(command);
   if (i<8)  return two;
   if (i<16) return one;
   return zero;
}

/*
 * orcacomment -- return the start of the comment field in a line of
 *                the Orca SYSCMND file.  This _must_ be called before
 *                orcacommand() is called on the same buffer, because
 *                orcacommand() modifies the buffer.
 */

static char *orcacomment (char *buffer) {
   char *p;
   static char *bad_line="malformed SYSCMND line\n";

   p = buffer;

   /* get past the command name */
   while ((*p != ' ') && (*p != '\t') && *p) p++;
   if (!*p) return bad_line;

   /* get past space */
   while ((*p == ' ') || (*p == '\t') && *p) p++;
   if (!*p) return bad_line;

   if (*p == '*') p++;
   switch (*p) {
      case 'U':   /*FALLTHROUGH*/
      case 'u':
         p++;
         break;
      case 'C':   /*FALLTHROUGH*/
      case 'L':   /*FALLTHROUGH*/
      case 'c':   /*FALLTHROUGH*/
      case 'l':
         p++;
         
         /* get past space */
         while ((*p == ' ') || (*p == '\t') && *p) p++;
         if (!*p) return bad_line;
         
         /* get past command or language number */
         while (isdigit(*p)) p++;

         break;
      default:
         return bad_line;
         break;
   }      

   /* get past any remaining space */
   while ((*p == ' ') || (*p == '\t')) p++;
   if (!*p) return bad_line;
   return p;
}                                          

/*
 * orcacommand -- returns the command name that starts the buffer.
 *                This _must_ be called after orcacomment because it
 *                modifies the buffer.
 */

static char *orcacommand(char *buffer) {
   char *p;

   p = buffer;
   while (*p && !isspace(*p)) p++;
   *p = '\0';
   return buffer;
}

/*
 * apropos -- print matching lines from WHATIS database.  Returns the
 *            number of matching lines, or -1 on error.
 *
 *            Argv is an array of argc strings, each of which is a keyword.
 *            Apropos will print out any line that matches any of the
 *            keywords in argv.  The match algorithm depends on the value
 *            of apropos_mode, which may be one of the following.  For the
 *            first three modes, global variable <manpath> must be set.
 *
 *               MAN_K_MODE  -- this is the one used by apropos(1).  A
 *                              match is considered to be made if the keyword
 *                              appears anywhere in the WHATIS database line.
 *                              The match is case insensitive.
 *
 *               MAN_F_MODE  -- Any leading path component is stripped from
 *                              the keywords, and then a match is attempted
 *                              for any part of the WHATIS database line.  The
 *                              match is case insensitive.
 *
 *               WHATIS_MODE -- The WHATIS database line is parsed up to the
 *                              first occurance of a '(' character.  If any
 *                              of the keywords are found, then the line is
 *                              printed.  The search is case sensitive.
 *
 *               ORCA_K_MODE -- Like MAN_K_MODE but instead of checking
 *                              the various WHATIS databases, it checks
 *                              15/syscmnd.
 *                                                 
 *               ORCA_W_MODE -- Like WHATIS_MODE but instead of checking
 *                              the various WHATIS databases, it checks
 *                              15/syscmnd.
 *
 * WARNING:  Use of the MAN_F_MODE may alter the contents of argv[].
 */

int  apropos(int argc, char **argv, int apropos_mode) {
   char **manpath_array;
   char *current_path, *p, *q, *r, *keyword;
   FILE *fp;
   char dirbrk;
   int i, j, matches;
   
#ifdef DEBUG
   assert ((apropos_mode == WHATIS_MODE) ||
           (apropos_mode == MAN_K_MODE)  ||
           (apropos_mode == MAN_F_MODE)  ||
           (apropos_mode == ORCA_K_MODE) ||
           (apropos_mode == ORCA_F_MODE) ||
           (apropos_mode == ORCA_W_MODE));
#endif

   matches = 0;

   /*
    * if we're doing 'man -f', get the basename for all keywords
    */
   if ((apropos_mode == MAN_F_MODE) || (apropos_mode == ORCA_F_MODE)) {
      for (i=0; i<argc; i++) {
         dirbrk = (strchr(argv[i],':')!=NULL) ? ':' : '/';
         if ((q = strrchr(argv[i],dirbrk)) != NULL) {
            q++;
            p = argv[i];
            do {
               *p = *q;
               p++; q++;
            } while (*q);
            *p = '\0';
         }
      }
   }


   if ((apropos_mode == ORCA_K_MODE) ||
       (apropos_mode == ORCA_W_MODE) ||
       (apropos_mode == ORCA_F_MODE)) {
      
      /*
       * searching the Orca 15/syscmnd file
       */

      if ((fp = fopen(SYSCMND,"r")) == NULL) {
         fprintf(stderr,"couldn't open %s\n",SYSCMND);
         return -1;
      }

      /* loop over lines in file */
      while ((q = fgets(linebuf,BUFFERSIZE,fp)) != NULL) {
         if (*linebuf == ';') continue;

         /* loop over keywords */
         for(i=0; i<argc; i++) {
            keyword = argv[i];
            if ((apropos_mode == ORCA_K_MODE && ncstrstr(linebuf,keyword)) ||
                (apropos_mode == ORCA_F_MODE && strstr(linebuf,keyword)) ||
                (apropos_mode == ORCA_W_MODE &&
                 !strncmp(linebuf,keyword,strlen(keyword)))) {
               r = orcacomment(linebuf);
               p = orcacommand(linebuf);
               q = orcapadding(p);
               printf("%s (Orca) %s- %s",p,q,r);
               matches++;
            }
         }
      }
      fclose(fp);
      return matches;
   }
      
   if ((manpath_array = makePathArray(manpath)) == NULL) return -1;

   /*
    * loop over all the paths in MANPATH
    */
   i=0;
   current_path = manpath_array[i];
   while (current_path) {

      dirbrk = (strchr(current_path,':')!=NULL) ? ':' : '/';
      if (chdir(current_path) == -1) {
         fprintf(stderr,"%s: %s\n",current_path,strerror(errno));
      } else if ((fp=fopen(WHATIS,"r"))==NULL) {
         if (access(WHATIS,F_OK) == -1) {
            fprintf(stderr,"%s%c%s: %s\n",current_path,dirbrk,WHATIS,
                    strerror(ENOENT));
         } else {
            fprintf(stderr,"error opening %s%c%s\n",current_path,dirbrk,
                    WHATIS);
         }
      } else {
         /* read each line, looking for a match */
         for (;;) {
            q = fgets(linebuf,BUFFERSIZE,fp);
            if (q != NULL) {

               /* search for a match to any keyword */
               for (j=0; j<argc; j++) {
                  keyword = argv[j];
                  switch (apropos_mode) {
                  case MAN_F_MODE:
                     if (strstr(linebuf,keyword)) {
                        printf("%s",linebuf);
                        matches++;
                     }
                     break;
                  case MAN_K_MODE:
                     if (ncstrstr(linebuf,keyword)) {
                        printf("%s",linebuf);
                        matches++;
                     }            
                     break;
                  case WHATIS_MODE:
                     /* avoid unnecessary strcpy's */
                     if (strstr(linebuf,keyword)==NULL) break;
                     strcpy(linebuf2,linebuf);
                     if ((p = strchr(linebuf2,'(')) != NULL) *p = '\0';
                     if (strstr(linebuf2,keyword)) {
                        printf("%s",linebuf);
                        matches++;
                     }            
                     break;                        
                  default:
                     fprintf(stderr,"internal error line %d of %s\n",
                             __LINE__,__FILE__);
                     exit(1);
                  }
               }
            } else if (ferror(fp)) {
               fprintf(stderr,"error reading %s database: %s\n",WHATIS,
                       strerror(errno));
               break;
            } else break;
         }
         fclose(fp);
      }

      i++;
      current_path = manpath_array[i];
   } /* endwhile loop over directories */

   return matches;
}
