/*
 * Copyright 1995 by Devin Reade <gdr@myrias.com>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 */

segment "catman____";

#include <sys/types.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <libc.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>
#include "util.h"
#include "man.h"

#define OVERFLOW_ABORT(line,file) { \
   fprintf(stderr,overflowMsg,line,file); \
   exit(1); \
}

short v_flag, V_flag, M_flag, m_flag, p_flag, err_flag;

extern int  optind;
extern char *optarg;

static char *versionstr = "1.0";

/* This is of the form "man<section>" */
char mandir[FILENAME_MAX];

/*
 * These are of the form
 *    "(man|cat)<section>/<name>.<section>[.<compression_suffix>]"
 */
char manfile[FILENAME_MAX];   
char catfile[FILENAME_MAX];
char catfile2[FILENAME_MAX];

char base[FILENAME_MAX];

static char *overflowMsg = "internal buffer overflow at line %d of %s\n";

extern void begin_stack_check(void);
extern int  end_stack_check(void);

/*
 * catman
 *
 * Pre:  argv is an array of section numbers (character strings), possibly
 *          empty.
 *       argc is the number of entries in argv.
 *       global variable <manpath> is a space- or colon-delimited list
 *          of pathnames.
 *
 * Post: for each pathname in <manpath>, catman will preformat the manpages
 *       within the man* subdirectories and place the result in the
 *       corresponding cat* subdirectory.  If section numbers are specified,
 *       only those sections will be done.
 *
 *       Returns 0 on success, 1 on failure.
 */

int catman(int argc, char **argv) {

   char **manpath_array;   /* MANPATH components in array form */
   char *current_path;     /* the current MANPATH component */
   int i;                  /* an index */
   int pathIndex;          /* offset into manpath_array[] */
   int sectionIndex;       /* offset into sections[] */
   DIR *manp, *catp;       /* directory pointers */
   struct dirent *entp;    /* current file entry in manp */
   char *sec;              /* the current section "number" */
   int catfile_found;      /* have we found a preformatted version */
                           /* that's newer than the unformatted version? */
   fileType *ftype;        /* the file type of the man page */
   struct stat statbuf1, statbuf2;
   char dirbrk;

   /* create array of paths to search */
   if ((manpath_array = makePathArray(manpath)) == NULL) return 1;

   /* loop over paths in MANPATH */
   pathIndex=0;
   current_path = manpath_array[pathIndex];
   while(current_path) {

      dirbrk = (strchr(current_path,':')==NULL) ? '/' : ':';

      /* go to the current path in MANPATH */
      if (v_flag) printf("cd %s\n",current_path);
      if (chdir(current_path) == -1) {
         pathIndex++;
         current_path = manpath_array[pathIndex];
         continue;
      }


      /* loop over sections */
      for (sectionIndex=0; sections[sectionIndex].name!=NULL; sectionIndex++){

         /*
          * if section number was specified and this isn't it, do
          * the next loop
          */
         if (argc > 0) {
            sec = NULL;
            for (i=0; i<argc; i++) {
               if (!strcmp(argv[i],sections[sectionIndex].name)) {
                  sec = argv[i];
                  break;
               }
            }
            if (sec == NULL) continue;
         } else sec = sections[sectionIndex].name;

#ifdef DEBUG
         assert(sec);
#endif

         /*
          * we're going to update this section.  Open the man? subdir
          */

         if (strlen(sec) + 3 >= FILENAME_MAX) {
            OVERFLOW_ABORT(__LINE__,__FILE__);
         }
         sprintf(mandir,"man%s",sec);
         if ((manp = opendir(mandir)) == NULL) continue;

         /* make sure the cat? directory exists, but leave it closed */
         sprintf(catfile,"cat%s",sec);
         if ((catp = opendir(catfile)) == NULL) {
            closedir(manp);
            continue;
         } else {
            closedir(catp);
         }

         /* loop over files in this section */
         while ((entp = readdir(manp)) != NULL) {

            /* skip standard file entries */
            if (!strcmp(entp->d_name,".") || !strcmp(entp->d_name,"..")) {
               continue;
            }

            /*
             * make the base the name of the man? entry excluding
             * any compression suffix
             */
            strcpy(base,entp->d_name);
            for (i=0; compressArray[i].suffix != NULL; i++) {
               size_t len1, len2;

               len1 = strlen(compressArray[i].suffix);
               len2 = strlen(base);
               if (!strcmp(compressArray[i].suffix,
                   (char *)(base + len2 - len1))) {
                  *(base + len2 - len1) = '\0';
                  break;
               }
            }

            /*
             * set manfile and catfile properly
             */

            sprintf(manfile,"man%s%c%s",sec,dirbrk,entp->d_name);
            sprintf(catfile,"cat%s%c%s",sec,dirbrk,base);
            
            if (stat(manfile,&statbuf1) != 0) {
               fprintf(stderr,"stat on %s failed: %s: file skipped\n",
                  manfile, strerror(errno));
               continue;
            }

            /*
             * search for an uncompressed file in the cat? directory,
             * unlinking any older files.
             */
            catfile_found=0;
            if (stat(catfile,&statbuf2) == 0) {
               if (statbuf1.st_mtime <= statbuf2.st_mtime) {
                  catfile_found++;          
               } else {
                  if (v_flag) printf("rm %s%c%s\n",current_path,dirbrk,catfile);
                  if (!p_flag) unlink(catfile);
               }
            }

            /*
             * search for any compressed files in the cat? directory,
             * unlinking any older files.
             */
            for(i=0;compressArray[i].suffix != NULL; i++) {
               sprintf(catfile2,"cat%s%c%s%s",sec,dirbrk,base,
                  compressArray[i].suffix);
               if (stat(catfile2,&statbuf2) == 0) {
                  if ((!catfile_found) &&
                      (statbuf1.st_mtime <= statbuf2.st_mtime)) {
                     strcpy(catfile,catfile2);
                     catfile_found++;
                  } else {
                     if (v_flag)
                        printf("rm %s%c%s\n",current_path,dirbrk,catfile2);
                     if (!p_flag) unlink(catfile2);
                  }
               }
            }
            if (catfile_found) continue;

            /*
             * If we got to this point, then we will be preformatting
             * the manual page.  There is also no version of the man
             * page left in cat?.
             */
            
            sprintf(catfile,"cat%s%c%s",sec,dirbrk,base);
            i = getSuffixIndex(manfile);
            if (i >= 0) {
               if (strlen(compressArray[i].extractor) + strlen(manfile) +
                   strlen(NROFF) + strlen(catfile) + 14 >= BUFFERSIZE) {
                  OVERFLOW_ABORT(__LINE__,__FILE__);
               }
               sprintf(linebuf,"%s %s | %s -man - > %s",
                  compressArray[i].extractor,manfile,NROFF,catfile);
            } else {

               /* determine which roffer to use based on the file type */
               if ((ftype = getFileType(manfile)) == NULL) {
                  fprintf(stderr,"getFileType failed for %s: %s\n",
                     manfile,strerror(errno));
                  continue;
               }
               if ((ftype->type == 0x50) && (ftype->auxtype == 0x8010)) {
                  if (strlen(AROFF) + strlen(manfile) + strlen(catfile)
                      >= BUFFERSIZE) {
                     OVERFLOW_ABORT(__LINE__,__FILE__);
                  }
                  sprintf(linebuf,"%s %s > %s",AROFF,manfile,catfile);
               } else if ((ftype->type == TXT) || (ftype->type == BIN) ||
                          (ftype->type == SRC) || (ftype->type == NON)) {
                  if (strlen(NROFF) + strlen(manfile) + strlen(catfile)
                      >= BUFFERSIZE) {
                     OVERFLOW_ABORT(__LINE__,__FILE__);
                  }
                  sprintf(linebuf,"%s -man %s > %s",NROFF,manfile,catfile);
               } else {
                  fprintf(stderr,"illegal file type for %s: file skipped\n",
                     manfile);
                  continue;
               }
            }
            if (v_flag) printf("%s\n",linebuf);
            if (!p_flag) system(linebuf);
         }  /* done looping over files */
         closedir(manp);
      }  /* done looping over sections */

      /* set up for the next component of MANPATH */
      pathIndex++;
      current_path = manpath_array[pathIndex];
   }
}


int main (int argc, char **argv) {
   char *path;
   int i, result1, result2;

   /* make sure Gno is running */
   if (needsgno()==0) {
      fprintf(stderr,"Requires Gno/ME\n");
      return 1;
   }

#ifdef STACK_CHECK
   begin_stack_check();
#endif

   /* initialization */
   v_flag = V_flag = M_flag = m_flag = p_flag = err_flag = 0;
   result1 = result2 = 0;

   /* parse command line and check usage */
   while((i = getopt(argc,argv,"M:m:pVv")) != EOF) {
      switch(i) {
      case 'M':
         if (m_flag) err_flag++;
         M_flag++;
         path = optarg;
         break;
      case 'm':
         if (M_flag) err_flag++;
         m_flag++;
         path = optarg;
         break;
      case 'p':
         p_flag++;
         v_flag++;
         break;
      case 'V':
         V_flag++;
         break;
      case 'v':
         v_flag++;
         break;
      default:                         
         err_flag++;
      }
   }
   if (err_flag || V_flag) {
      fprintf(stderr,"%s version %s by Devin Reade\n",
              basename(argv[0]),versionstr);
   }
   if (err_flag) {
      fprintf(stderr,"Usage: %s [-pVv] [-M path] [-m path] [section ...]\n",
              basename(argv[0]));
   }
   if (err_flag || V_flag) return 1;

   /* translate selected "sections" into something more understandable */
   for (i=optind; i<argc; i++) {
      if (!strcmp(argv[i],"local"))  argv[i] = "l";
      if (!strcmp(argv[i],"new"))    argv[i] = "n";
      if (!strcmp(argv[i],"old"))    argv[i] = "o";
      if (!strcmp(argv[i],"public")) argv[i] = "p";
   }

   /* do the search */
   if (M_flag) {
      manpath = path;
   } else {
      manpath = getManpath();
   }
   result1 = catman(argc-optind, &argv[optind]);
   if (!M_flag) free(manpath);
   if (m_flag) {
      manpath = path;
      result2 = catman(argc-optind, &argv[optind]);
   }

#ifdef STACK_CHECK
   fprintf(stderr,"stack usage: %d bytes\n",end_stack_check());
#endif

   return (result1 || result2);
}
