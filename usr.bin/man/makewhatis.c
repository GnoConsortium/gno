/*
 * Copyright 1995-1998 by Devin Reade <gdr@trenco.gno.org>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 *
 * $Id: makewhatis.c,v 1.4 1998/03/29 07:16:01 gdr-ftp Exp $
 */

#ifdef __ORCAC__
segment "makewhatis";
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <gno/gno.h>
#include "man.h"

/*
 * Options:
 *
 * -c             check cat* subdirectories as well
 * -C             check _only_ cat* subdirectories, not man* subdirectories.
 * -f outfile     force whatis status output to <outfile>
 * -l logfile     log errors to <logfile>
 * -o dbfile      force whatis database output to <dbfile>
 * -p path        use the <path> rather than $MANPATH, $USRMAN, or $MANDIR
 * -s             sort the whatis database by manual page name
 * -v{1|2|3}      verbose
 * -V             show version and usage info and exit
 */

char *man_subdir[] = {
   "man1",
   "man2",
   "man3",
   "man3f",
   "man4",
   "man5",
   "man6",
   "man7",
   "man8",
   "mann",
   "manl",
   "manp",
   "mano",
   NULL 
};

/*
 * we include cat* since some GNO utility man pages aren't written in
 * either nroff or aroff source ... go figure.
 */

char *cat_subdir[] = {
   "cat1",
   "cat2",
   "cat3f",
   "cat4",
   "cat5",
   "cat6",
   "cat7",
   "cat8",
   "catn",
   "catl",
   "catp",
   "cato",
   NULL           /* _must_ be NULL terminated! */
};

/* For the various command line flags */

short c_flag  = 0;
short C_flag  = 0;
short o_flag  = 0;
short p_flag  = 0;
short v_flag  = 0;
short errflag = 0;   /* This is set if there is a usage error or -V flag */

static char filebuffer[FILENAME_MAX];  /* used when traversing cat* pages */
static char progdir[FILENAME_MAX];  /* the directory where makewhatis started */

FILE *output_fp;
FILE *error_fp;

int main (int argc, char **argv) {
   static char tmp_file[L_tmpnam];   /* a scratch file */
   char *manpath;       /* the location of the man pages */
   char *path;          /* the current path; taken from manpath */
   char *p=NULL;        /* a temporary pointer */
   struct dirent *file; /* the current file we have open */
   FILE *tmp_fp;              /* pointer to tmp_file */
   FILE *whatis_fp;     /* pointer to the current whatis database */
   DIR  *subdir;        /* the current man subdirectory -- eg: /usr/man/man3 */
   char *dbfile;        /* non-default name of the whatis database */
   char *dirsep;        /* the directory separator, either "/" or ":" */
   int   i;
   extern int optind;
   extern char *optarg;

   /* make sure GNO is running */
   if (needsgno()==0) {
      fprintf(stderr,"Requires GNO\n");
      return 1;
   }

   __REPORT_STACK();

   /*
    * set the defaults
    */

   output_fp = stdout;
   error_fp  = stderr;
   if (getwd(progdir) == NULL) {
      perror("getwd() failed");
      exit(1);
   }

   /*
    * parse the command line
    */

   while((i = getopt(argc,argv,"cCf:l:o:p:v:V")) != EOF)
      switch(i) {
      case 'c':
         if (C_flag) errflag++;
         else c_flag++;
         break;
      case 'C':
         if (c_flag) errflag++;
         else C_flag++;
         break;
      case 'f':
         output_fp = fopen (optarg,"w");
         if (output_fp == NULL) {
            fprintf(stderr,"Could not open output file %s; using stdout.\n",
                    optarg);
            output_fp = stdout;
         }
         break;
      case 'l':
         error_fp = fopen (optarg,"w");
         if (error_fp == NULL) {
            fprintf(stderr,"Could not open log file %s; using stderr.\n",
                    optarg);
            error_fp = stderr;
         }
         break;
      case 'o':
         o_flag++;
         dbfile = optarg;
         break;
      case 'p':
         p_flag++;
         p = optarg;
         break;
      case 'v':
         v_flag = (short) atoi(optarg);
         if ((v_flag<1) && (v_flag>3)) errflag++;
         break;
      case 'V':
         fprintf(stderr,
            "%s --\n\tCreate the %s database.\n\tVersion %s by Devin Reade\n\n",
            argv[0],WHATIS,VERSION_STR);
         errflag++;
         break;
      default:
         errflag++;
         break;
      }
   if (errflag) {
      fprintf(error_fp,
"Usage:\n%s\t[-c|-C] [-f outfile] [-l logfile] [-o dbfile] [-p path]\n\
\t\t[-v 1|2] [-V]\n\n\
-c\t\tCheck catX subdirectories as well\n\
-C\t\tCheck _only_ catX subdirectories, not manX subdirectories.\n\
-f outfile\tForce whatis output to <outfile>.\n\
-l logfile\tLog errors to <logfile>.\n\
-o dbfile\tforce whatis database output to <dbfile>.\n\
-p path\t\tUse the <path> rather than $MANPATH, $USRMAN, or $MANDIR.\n\
-v n\t\t<n>=1: Slightly verbose, only displaying major errors.\n\
\t\t<n>=2: Verbose, displaying processed file names.\n\
\t\t<n>=3: Very verbose, displaying more processing info.\n\
-V\t\tShow version and usage information, then exit.\n",argv[0]);
      return -1;
   }
                                                  
   /*
    * get the location of the man pages; p is already set if -p flag used
    */

   if (p == NULL) p = getenv("MANPATH");
   if (p == NULL) p = getenv("USRMAN");
   if (p == NULL) p = getenv("MANDIR");
   if (p == NULL) p = DEFAULT_MANPATH;

   /* define the directory separator */
   dirsep = (strchr(p,':')==NULL) ? "/" : ":";

   /* make a copy of the location */
   if ((manpath = malloc (strlen(p) + 1)) == NULL) {
      if (v_flag) fprintf(error_fp,
                  "malloc failed while making copy of \"%s\"\nAborted.\n",p);
      return -1;
   }
   strcpy(manpath,p);

   /* get the name of the temporary file */
   tmpnam (tmp_file);

   /*
    * loop over all the paths in manpath. Don't use ':' as a delimiter since
    * the colon is a pathname separator under GS/OS.
    */

   path = strtok (manpath," ");
   while (path != NULL) {

      if (access(path,F_OK)==0) {

         if (strcmp(path,".") == 0) {
            chdir(progdir);
         } else {
            chdir(path);
         }

         /* open the whatis database file */
         if (!o_flag) dbfile = WHATIS;
         whatis_fp = fopen(dbfile,"w");
         if (whatis_fp == NULL) {
            fprintf (error_fp,
               "Could not create whatis database file %s in directory %s.\n\
Aborted.\n",
               dbfile,path);
            exit(-1);
         }

         /*
          * loop over the expected man* subdirectories within path
          */

         if (!C_flag) for (i=0; man_subdir[i] != NULL; i++) {
            subdir = opendir(man_subdir[i]);
            if (subdir != NULL) {

               /* print status */
               if (v_flag>=3) fprintf(output_fp,
                  "Now working on directory %s\t%s ...\n",path,man_subdir[i]);

               /* no need to error check because of opendir() */
               chdir(man_subdir[i]);

               /* loop over files within subdirectory */
               while ((file = readdir(subdir)) != NULL) {
                  process (file->d_name,tmp_file,whatis_fp,&man_subdir[i][3]);
               }
               closedir(subdir);
            } else {
               if (v_flag>=3) fprintf(output_fp,
                  "Could not access files in %s\t%s ...\n",path,man_subdir[i]);
            }
            chdir(path);
         }
               
         /*
          * loop over the expected cat* subdirectories within path,
          * adding only those files without man* versions.
          */

         if (c_flag||C_flag) for (i=0; cat_subdir[i] != NULL; i++) {
            subdir = opendir(cat_subdir[i]);
            if (subdir != NULL) {

               /* print status */
               if (v_flag>=3) fprintf(output_fp,
                  "Now working on directory %s\t%s ...\n",path,cat_subdir[i]);

               /* no need to error check because of opendir() */
               chdir(cat_subdir[i]);

               /* make filebuffer contain path to matching man* subdirectory */
               strcpy(filebuffer,path);
               strcat(filebuffer,dirsep);
               strcat(filebuffer,man_subdir[i]);
               strcat(filebuffer,dirsep);
               p = filebuffer + strlen(filebuffer); /* p points to '\0' */

               /* loop over files that don't have a man* counterpart */
               while ((file = readdir(subdir)) != NULL) {
                  strcpy(p,file->d_name);
                  if (access(filebuffer,F_OK)!=0)
                     process (file->d_name,tmp_file,whatis_fp,
                        &man_subdir[i][3]);
               }
               closedir(subdir);
            } else {
               if (v_flag>=3) fprintf(output_fp,
                  "Could not access files in %s\t%s ...\n",path,cat_subdir[i]);
            }
            chdir(path);
         }
               
         /* close the database */
         fclose(whatis_fp);

      }
      /* get the next path in manpath */
      path = strtok (NULL," ");
   }


   /* clean up and exit */                         
   unlink(tmp_file);
   if (output_fp != stdout) fclose(output_fp);
   if (error_fp != stderr)  fclose(error_fp);

   return 0;
}
