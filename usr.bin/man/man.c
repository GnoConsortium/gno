/*
 * Copyright 1995-1998 by Devin Reade <gdr@trenco.gno.org>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 *
 * $Id: man.c,v 1.2 1998/03/29 07:16:07 gdr-ftp Exp $
 */

#ifdef __ORCAC__
segment "man_______";
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <sgtty.h>
#include <unistd.h>
#include <err.h>
#include <gno/gno.h>
#include "man.h"

char *macroPackage   = "an";              /* default /usr/lib/tmac/tmac.an */
char *versionStr     = VERSION_STR;
char *pager;
char *troff;
char *tcat;

static char *nothing = "nothing appropriate";

short f_flag, k_flag, M_flag, n_flag, t_flag, T_flag, V_flag;
short hyphen_flag, err_flag;

static void usage(char *progname) {
   if (err_flag || V_flag) fprintf(stderr,"%s %s\n",progname,versionStr);
   if (err_flag) {
      fprintf(stderr,"Usage:\n\
\t%s [-] [-nt] [-M path] [-T macro-package] [section] title ...\n\
\t%s [-n] [-M path] -f filename ...\n\
\t%s [-n] [-M path] -k keyword ...\n\n",progname,progname,progname);
      exit(1);
   }
   return;
}

int main (int argc, char **argv) {
   int i, result;
   char *p;

   extern int optind;
   extern char *optarg;

   /* make sure GNO is running */
   if (needsgno()==0) {
	err(1, "Requires GNO");
   }

   __REPORT_STACK();

   /*
    * initialization
    */

   f_flag = k_flag = M_flag = t_flag = T_flag = V_flag = 0;
   n_flag = hyphen_flag = err_flag = 0;

   /*
    * parse the command line
    */

   while((i = getopt(argc,argv,"fkM:ntT:V-")) != EOF) {
      switch(i) {
      case 'f':
         f_flag++;
         if (k_flag || t_flag || T_flag || hyphen_flag) err_flag++;
         break;
      case 'k':
         k_flag++;
         if (f_flag || t_flag || T_flag || hyphen_flag) err_flag++;
         break;                            
      case 'M':
         M_flag++;
         manpath = LC_xstrdup(optarg);
         break;
      case 'n':
         n_flag++;
         break;
      case 't':
         t_flag++;
         if (k_flag || f_flag) err_flag++;
         break;
      case 'T':
         T_flag++;
         macroPackage = optarg;
         if (k_flag || f_flag) err_flag++;
         break;              
      case 'V':
         V_flag++;
         break;
      default:
         err_flag++;
      }
   }
   /* take care of the '-' option, since getopt isn't smart enough */
   for (i=optind; i<argc; i++) {
      if (strcmp(argv[i],"-") == 0) {
         hyphen_flag++;
         if (k_flag || f_flag) err_flag++;
         --argc;
         for( ; i<argc; i++) {
            argv[i] = argv[i+1];
         }
      }
   }

   if ((argc == optind) || (!k_flag && !f_flag && (argc-optind > 3))) {
      err_flag++;
   }

   usage(argv[0]);

   /* if not already done, set the manpath */
   if (!M_flag) manpath = getManpath();

   i = 0;
   if (k_flag || f_flag) {
      result = apropos(argc-optind, &argv[optind],
                       ((k_flag) ? MAN_K_MODE : MAN_F_MODE));
      if (!n_flag) {
         i = apropos(argc-optind, &argv[optind],
                     ((k_flag) ? ORCA_K_MODE : ORCA_F_MODE));
      }
      if (result<=0 && i<=0) {
         fprintf(stderr,"%s: %s\n",basename(argv[0]),nothing);
      }
   } else {
      if (!isatty(STDOUT_FILENO)) hyphen_flag++;
      result = man(argc-optind, &argv[optind]);
   }
   free(manpath);
   result = ((result == 0) && (i == 0)) ? 0 : 1;
   return result;
}
