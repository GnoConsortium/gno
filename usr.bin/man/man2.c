/*
 * Copyright 1995-1998 by Devin Reade <gdr@trenco.gno.org>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 *
 * $Id: man2.c,v 1.2 1998/03/29 07:16:12 gdr-ftp Exp $
 */

#ifdef __ORCAC__
segment "man2______";
#pragma noroot
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sgtty.h>
#include <err.h>
#include <gno/contrib.h>
#include "man.h"

#define MAX(a,b) ((a) > (b)) ? (a) : (b)

static LC_StringArray_t	buildManList(char *suffix, char *name);
static LC_StringArray_t makePathArray(const char *manpath);
static void		display(char *list);
static char *		getBaseName (char *out, char *in);

/*
 * Pre:  argc is the number of strings in argv.  It should either be 1 or 2.
 *          If DEBUG is defined, then any other value of argc will result in
 *          a failed assert.
 *
 *       argv is an array of strings.  If argc==1, then argv[0] is presumed
 *          to be the name of the man page.  If argc==2, then argv[0] is
 *          presumed to be a section number and argv[1] is the name of the
 *          man page.
 *
 *       globals:
 *          manpath must be a malloc'd string.  Either ':' or ' ' may
 *             may be used as a directory delimiter.
 *          macroPackage must be set to a static string.
 *          t_flag must be in a defined state (either set or unset)
 *          hyphen_flag must be in a defined state (either set or unset)
 *
 * Post: print the man page in whatever form it takes, acting as necessary
 *       on the -, -M, and -T flags and section number.
 */

int man(int argc, char *argv[]) {
   LC_StringArray_t manpath_array, manpagelist;
   char *sec, *name, *current_path;
   Section *section;
   int i, j, k, abort;
   short section_found, page_found;
   char dirbrk; /* path component separator.  either ':' or '/' */
   char c, *p;
   struct sgttyb termMode;
   short oldMode;
   
   /* initialization */
   abort = 0;
   section_found = 0;
   page_found = 0;
   if (t_flag) {
      if ((tcat = getenv("TCAT")) == NULL) {
         tcat = TCAT;
      } else tcat = LC_xstrdup(tcat);
      if ((troff = getenv("TROFF")) == NULL) {
         troff = TROFF;
      } else troff = LC_xstrdup(troff);
   }
   if (hyphen_flag) {
      pager = CAT;
   } else if ((pager = getenv("PAGER")) == NULL) {
      pager = PAGER;
   }

   /* determine name and, if appropriate, sec */
   switch (argc) {
   case 1:
      sec = NULL;
      name = argv[0];
      break;
   case 2:
      sec = argv[0];
      /* special case some section abbreviations */
      if (!strcasecmp(sec,"l")) {
         sec = "local";
      } else if (!strcasecmp(sec,"n")) {
         sec = "new";
      } else if (!strcasecmp(sec,"o")) {
         sec = "old";
      } else if (!strcasecmp(sec,"p")) {
         sec = "public";
      }
      name = argv[1];
      break;
   default:
      fprintf(stderr,"internal error at line %d in file %s\n",__LINE__,
              __FILE__);
      exit(1);
   }

   /* create array of paths to search */
   if ((manpath_array = makePathArray(manpath)) == NULL) {
	return 1;
   }

   /*
    * loop over all the paths in MANPATH
    */
   i=0;
   current_path = (manpath_array->lc_vec)[i];
   while (!abort && current_path) {

      dirbrk = (strchr(current_path,':')!=NULL) ? ':' : '/';

      /* go to the current path in MANPATH */
      if (chdir(current_path) == -1) {
         i++;
         current_path = (manpath_array->lc_vec)[i];
         continue;
      }

      /* loop over sections */
      for (j=0; !abort && sections[j].name != NULL; j++) {

         /*
          * if section number was specified and this isn't it, do
          * the next loop
          */
         if (sec && (strcasecmp(sec,sections[j].name) ||
                     (!isdigit(*sec) &&
                      strncasecmp(sec,sections[j].name,strlen(sec))))) continue;
         section_found++;

         /*
          * we're going to check this section.  Get the pathnames
          * (relative to this directory) of the two files.
          */

         manpagelist = buildManList(sections[j].suffix,name);
         if (manpagelist->lc_used == 0) continue;
         page_found++;

         for (k=0; !abort && k < manpagelist->lc_used; k++) {
            display((manpagelist->lc_vec)[k]);
            if (!hyphen_flag && !t_flag) {
               fprintf(stderr,
                  "type q to quit, or any other key for next man page: ");
               c = getcharraw();
               if (c == '\0') {
                  fprintf(stderr,"getcharraw failed line %d of %s: %s\n",
                     __LINE__,__FILE__,strerror(errno));
                  exit(1);
               }
               fputc('\n',stderr);
               /* evaluate result */
               if ((c == 'q') || (c == 'Q')) abort++;
            }
         }

         LC_StringArrayDestroy(manpagelist);

      }   /* done looping over sections */
      i++;
      current_path = (manpath_array->lc_vec)[i];

   }  /* done looping over paths */

   i = 0; /* used here for the return code */
   if (sec) {
      if (!section_found) {
         fprintf(stderr,"there is no section %s in the manual\n",sec);
         i++;
      } else if (!page_found) {
         fprintf(stderr,"there is no %s in section %s\n",name,sec);
         i++;
      }
   } else if (!page_found) {
      fprintf(stderr,"there is no %s in the manual\n",name);
      i++;
   }
   return i;
}


#define DIRBUF_LEN 10
static const char *manstr="man";
static const char *catstr="cat";

static LC_StringArray_t
buildManList(char *suffix, char *name) {
   static char buffer1[FILENAME_MAX];
   static char buffer2[FILENAME_MAX];
   DIR *directory;
   struct dirent *entry;
   LC_StringArray_t list1, list2;
   size_t len;
   int i, j, k;
   char *p, *fn, *L1, *L2;

   /* initialization */
   list1 = LC_StringArrayNew();
   list2 = LC_StringArrayNew();

#ifdef DEBUG
   /* sanity check on arguments */
   if(MAX(strlen(manstr),strlen(catstr)) + strlen(suffix) >= FILENAME_MAX) {
      fprintf(stderr,"internal error: buffer overflow at line %d of %s\n",
              __LINE__,__FILE__);
   }
#endif

   /*
    * look in man subdirectory
    */
   strcpy(buffer1,manstr);
   strcat(buffer1,suffix);
   if ((directory = opendir(buffer1)) != NULL) {
      while ((entry = readdir(directory)) != (struct dirent *) NULL) {

         /* skip if no match */
         len = strlen(name);
         if (strncasecmp(entry->d_name,name,len) ||
             (entry->d_name[len] != '.')) continue;

         if (strlen(manstr) + strlen(suffix) + strlen(entry->d_name) +
             strlen(suffix) + 1 >= FILENAME_MAX) {
       		errx(1, "internal error: buffer overflow at line %s:%d\n",
                     __FILE__, __LINE__);
         }
         sprintf(buffer1,"%s%s:%s",manstr,suffix,entry->d_name);

         /* look for "links" to aroff files. (what a kludge) */
         if ((buffer1[3] != 'l') &&
             (strcasecmp(".l",&buffer1[strlen(buffer1)-2])==0)) {
            FILE *linkptr;
            char *tp;

            /* dereference the "link" */
            if ((linkptr = fopen(buffer1,"r")) == NULL) {
               fprintf(stderr,"couldn't open %s\n",buffer1);
            } else if (fgets(buffer2,FILENAME_MAX,linkptr)==NULL) {
               fprintf(stderr,"couldn't read %s\n",buffer1);
            } else {

               /* drop trailing space and newline */
               tp = buffer2 + strlen(buffer2) -1;
               while ((tp>=buffer2) && (isspace(*tp) || *tp == '\n')) {
                  *tp = '\0';
                  tp--;
               }
               fclose(linkptr);

               if (access(buffer2,R_OK) == 0) {
		  LC_StringArrayAdd(list1, buffer2);
               }
            }                                   
         } else {
            /* not a .l "link"; a normal file */
	    LC_StringArrayAdd(list1, buffer1);
         }
      }              
      closedir(directory);
   }

   if (!t_flag) {
      /*
       * look in cat subdirectory
       */
      strcpy(buffer1,catstr);
      strcat(buffer1,suffix);
      if ((directory = opendir(buffer1)) != NULL) {
         while ((entry = readdir(directory)) != (struct dirent *) NULL) {

            /* skip if no match */
            len = strlen(name);
            if (strncasecmp(entry->d_name,name,len) ||
                (entry->d_name[len] != '.')) continue;

            if (strlen(catstr) + strlen(suffix) + strlen(entry->d_name) +
                strlen(suffix) + 1 >= FILENAME_MAX) {
               fprintf(stderr,"internal error: buffer overflow at line %d of %s\n",
                       __LINE__,__FILE__);
            }
            sprintf(buffer1,"%s%s:%s",catstr,suffix,entry->d_name);
	    LC_StringArrayAdd(list2, buffer1);
         }              
         closedir(directory);
      }
   }

   /*
    * eliminate files common to both lists
    */
   len = strlen(suffix);
   for(i=0; i< list1->lc_used; i++) {
      L1 = (list1->lc_vec)[i];
      for (j=0; j< list2->lc_used; j++) {
         L2 = (list2->lc_vec)[j];

         getBaseName(buffer1, L1);
         getBaseName(buffer2, L2);

#ifdef DEBUG
         if ((strlen(buffer1) < len + 5) ||
             (strlen(buffer2) < len + 5)) {
		err(1, "internal error at line %d of %s\n", __LINE__, __FILE__);
         }
#endif
         /* match after the respective "manXX/" and "catXX/" */
         if (strcasecmp(&buffer1[len+4],&buffer2[len+4]) == 0 ) {
         
            p = newerFile(L1,L2);
            if (p == L1) {
		LC_StringArrayDelete(list2, L2);
		--j;
            } else if (p == (list2->lc_vec)[j]) {
		LC_StringArrayDelete(list1, L1);
		--i;
		break;
            } else {
	       err(1, "internal error at %s:%d (newerFile failed)",
		   __FILE__, __LINE__);
            }
         }  /* endif */
      }     /* endfor */
   }        /* endfor */

   /*
    * combine the two lists
    */
   j = list2->lc_used;
   for (i=0; i<j; i++) {
	LC_StringArrayAdd(list1, (list2->lc_vec)[i]);
   }
   LC_StringArrayDestroy(list2);
   return list1;
}

/*
 * display
 */

#define MANSUBDIR 0
#define CATSUBDIR 1

#define FORMAT   "Formatting manual page, please wait ..."
#define DECOMP   "Decompressing manual page, please wait ..."
#define DECFOR   "Decompressing and formatting manual page, please wait ..."
#define OVERFLOW "Internal buffer overflow ... aborted."

static void display(char *file) {
   int icompress, isubdir;
   char *tmac;
   char *roffer;
   char *compressor;
   fileType *ft;

#ifdef DEBUG
   if (file == NULL) {
      fprintf(stderr,"internal error line %d of %s\n",__LINE__,__FILE__);
      exit(1);
   }
   if (strlen(file) < 4) {
      fprintf(stderr,"internal error line %d of %s\n",__LINE__,__FILE__);
      exit(1);
   }
#endif                   

   /*
    * determine which subdirectory this file is in
    */
   if (strncasecmp(file,"cat",3) == 0) {
      isubdir = CATSUBDIR;
   } else {
      isubdir = MANSUBDIR;
   }

   /*
    * if we're troffing a new file to fmt?, make sure the directory
    * exists
    */
   if (t_flag && hyphen_flag) {
      char *p;
      struct stat sbuf;

      /* ensure the fmt? directory exists */
      sprintf(linebuf,"fmt%s",&file[3]);
      if ((p = strchr(linebuf,':')) != NULL) {
         *p = '\0';
#ifdef DEBUG
      } else if ((p = strchr(linebuf,'/')) == NULL) {
         fprintf(stderr,"internal error line %d of %s\n",__LINE__,
                 __FILE__);
         exit(1);
#endif
      } else *p = '\0';
      if (stat(linebuf,&sbuf) != 0) {
         fprintf(stderr,"couldn't stat %s: %s.  %s skipped\n",
                 linebuf,strerror(errno),file);
         return;
      } else if (!(sbuf.st_mode & S_IFDIR)) {
         fprintf(stderr,"cannot access %s: %s.  %s skipped\n",
                 linebuf,strerror(ENOTDIR),file);
         return;
      }
   }

   /*
    * determine the type of compression used, if any
    */
   icompress = getSuffixIndex(file);
   if (icompress >= 0) {
      compressor = compressArray[icompress].extractor;
      if (isubdir == MANSUBDIR) {
                                                
         /*
          * compressed nroff source
          */

         if (t_flag && hyphen_flag) {
            if (strlen(compressor) + 2*strlen(file) + strlen(troff) +
                strlen(macroPackage) + strlen(tcat) + 12 > BUFFERSIZE) {
               fprintf(stderr,"%s\n",OVERFLOW);
               exit(1);
            }
            sprintf(linebuf,"%s %s | %s -m%s - > fmt%s",
                    compressor, file, troff, macroPackage, &file[3]);
         } else if (t_flag) {
            if (strlen(compressor) + strlen(file) + strlen(troff) +
                strlen(macroPackage) + strlen(tcat) + 12 > BUFFERSIZE) {
               fprintf(stderr,"%s\n",OVERFLOW);
               exit(1);
            }
            sprintf(linebuf,"%s %s | %s -m%s - | %s",
                    compressor, file, troff, macroPackage, tcat);
         } else {
            /* not troff, jes' plain old nroff */
            if (strlen(compressor) + strlen(file) + strlen(NROFF) +
                strlen(macroPackage) + strlen(pager) + 12 > BUFFERSIZE) {
               fprintf(stderr,"%s\n",OVERFLOW);
               exit(1);
            }
            if (!hyphen_flag) printf("%s\n",DECFOR);
            sprintf(linebuf,"%s %s | %s -m%s - | %s",
                    compressor, file, NROFF, macroPackage, pager);
         }
      } else {

         /*
          * compressed straight text
          */

         if (strlen(compressor)+strlen(file)+strlen(pager)+5 > BUFFERSIZE) {
            fprintf(stderr,"%s\n",OVERFLOW);
            exit(1);
         }
         if (!hyphen_flag) printf("%s\n",DECOMP);
         sprintf(linebuf,"%s %s | %s", compressor, file, pager);
      }
   } else {
      if (isubdir == MANSUBDIR) {

         /*
          * Can be either aroff or nroff source.  If it's nroff source,
          * it must either be a TXT, SRC, or BIN file
          */

         if (!hyphen_flag && !t_flag) printf("%s\n",FORMAT);
         if ((ft = getFileType(file)) == NULL) {
            perror(file);
            exit(1);
         }
         if ((ft->type == 0x50) || (ft->auxtype == 0x8010)) {
   
            /*
             * AppleworksGS Word Processor format; use 'aroff'
             */

            if (t_flag) {
               fprintf(stderr,"cannot use troff on aroff source files\n");
               return;
            }
            if (strlen(AROFF)+strlen(file)+strlen(pager)+5  > BUFFERSIZE) {
               fprintf(stderr,"%s\n",OVERFLOW);
               exit(1);
            }
            sprintf(linebuf,"%s %s | %s", AROFF, file, pager);              
                   
         } else if ((ft->type == TXT) || (ft->type == BIN) ||
                    (ft->type == SRC) || (ft->type == NON)) {

            /*
             * TeXT, BINary, or SouRCe file; assume nroff source
             */
            if (t_flag && hyphen_flag) {
               if (strlen(troff) + strlen(macroPackage) +
                   2 * strlen(file) + 7 >= BUFFERSIZE) {
                  fprintf(stderr,"%s\n",OVERFLOW);
                  exit(1);
               }
               sprintf(linebuf,"%s -m%s %s > fmt%s",
                       troff, macroPackage, file, &file[3]);
            } else if (t_flag) {
               if (strlen(troff) + strlen(macroPackage) + strlen(file) +
                   strlen(tcat) + 7 >= BUFFERSIZE) {
                  fprintf(stderr,"%s\n",OVERFLOW);
                  exit(1);
               }
               sprintf(linebuf,"%s -m%s %s | %s",troff,macroPackage,file,tcat);
            } else {
               /* not troff, jes' plain old nroff */
               if (strlen(NROFF) + strlen(macroPackage) +
                  strlen(file) + strlen(pager) + 8 >= BUFFERSIZE) {
                  fprintf(stderr,"%s\n",OVERFLOW);
                  exit(1);
               }
               sprintf(linebuf,"%s -m%s %s | %s",NROFF,macroPackage,file,pager);
            }

         } else {
            fprintf(stderr,
                    "bad file type for %s\n\ttype = %x\n\taux  = %lx\n",
                    file,ft->type,ft->auxtype);
            return;
         }
      } else {
         /* assume straight text */
         if (strlen(CAT) + strlen(file) + strlen(pager) + 4 >= BUFFERSIZE){
            fprintf(stderr,"%s\n",OVERFLOW);
            exit(1);
         }
         sprintf(linebuf,"%s %s | %s", CAT, file, pager);
      }
   }   
   
#if 0
   fprintf(stderr,"DEBUG: BUFFER: %s\n",linebuf);
#endif
   system(linebuf);
   return;
}


/*
 * getBaseName -- copy the filename pointed to by <in> into the buffer
 *                pointed to by <out>, dropping any compression suffix
 *                that may be on the base name.  The set of compression
 *                suffixes is defined by the NULL-terminated compressArray[].
 *
 *                It is the caller's responsibility to ensure that the
 *                buffer *out has been allocated with sufficient space
 *                for the result.
 *
 *                Returns a pointer to <out>.
 */

static char *getBaseName (char *out, char *in) {
   char *p;
   int i;

   strcpy(out,in);
   if ((p = strrchr(out,'.')) != NULL) {
      for (i=0; compressArray[i].suffix; i++) {
         if (strcasecmp(p,compressArray[i].suffix)==0) {
            *p = '\0';
            break;
         }
      }
   }
   return out;
}

/*
 * makePathArray
 *
 * Pre:		<manpath> is a list of colon-delimited path names
 * Post:	returns a StringArray pointer where each string is an
 *		element of <manpath>
 */

static LC_StringArray_t
makePathArray(const char *manpath) {
	LC_StringArray_t result;
	char *pathcopy, *p;

	result = LC_StringArrayNew();
	pathcopy = LC_xstrdup(manpath);
	p = strtok(pathcopy, ":");
	while (p != NULL) {
		LC_StringArrayAdd(result, p);
		p = strtok(NULL, ":");
	}
	free(pathcopy);
	return result;
}
