/*
 * Copyright 1995-1998 by Devin Reade <gdr@trenco.gno.org>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 *
 * $Id: apropos.c,v 1.2 1998/03/29 07:15:42 gdr-ftp Exp $
 */

#ifdef __ORCAC__
segment "apropos___";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <gno/gno.h>
#include "man.h"

static char *versionstr = VERSION_STR;
static char *nothing    = "nothing appropriate";

int main (int argc, char **argv) {
   char *path;
   int i, matches1, matches2, matches3;
   short V_flag, M_flag, m_flag, n_flag, err_flag;

   /* make sure GNO is running */
   if (needsgno()==0) {
      errx(1, "Requires GNO\n");
   }

   __REPORT_STACK();

   /* initialization */
   V_flag = M_flag = m_flag = n_flag = err_flag = 0;
   matches1 = matches2 = matches3 = 0;

   /* parse command line and check usage */
   while((i = getopt(argc,argv,"M:m:nV")) != EOF) {
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
      case 'n':
         n_flag++;
         break;
      case 'V':
         V_flag++;
         break;
      default:
         err_flag++;
      }
   }
   if (argc-optind < 1) err_flag++;
   if (err_flag || V_flag) {
      fprintf(stderr,"%s version %s by Devin Reade\n",
              basename(argv[0]),versionstr);
   }
   if (err_flag) {
      fprintf(stderr,
         "Usage: %s [[-M path] | [-m path]] [-nV] keyword [keyword ...]\n",
         basename(argv[0]));
      return 1;
   }

   /* do the search */
   if (M_flag) {
      manpath = path;
   } else {
      manpath = getManpath();
   }
   matches1 = apropos(argc-optind, &argv[optind], MAN_K_MODE);
   if (!M_flag) free(manpath);
   if (m_flag) {
      manpath = path;
      matches2 = apropos(argc-optind, &argv[optind], MAN_K_MODE);
   }
   if (!n_flag) {
      matches3 = apropos(argc-optind, &argv[optind], ORCA_K_MODE);
   }

   i = 0;
   if (matches1>0) i+= matches1;
   if ( m_flag && matches2>0) i+=matches2;
   if (!n_flag && matches3>0) i+=matches3;

   if (i==0) {
      fprintf(stderr,"%s: %s\n",basename(argv[0]),nothing);
   }

   if ((matches1>=0) && (matches2>=0) && (matches3>=0) && i>0) return 0;
   return 1;
}
