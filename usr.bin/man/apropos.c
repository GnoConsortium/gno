/*
 * Copyright 1995 by Devin Reade <gdr@myrias.com>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 */

segment "apropos___";

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <libc.h>
#include "util.h"
#include "man.h"

extern int  optind;
extern char *optarg;

static char *versionstr = "3.0";
static char *nothing    = "nothing appropriate";

extern void begin_stack_check(void);
extern int  end_stack_check(void);

int main (int argc, char **argv) {
   char *path;
   int i, matches1, matches2, matches3;
   short V_flag, M_flag, m_flag, n_flag, err_flag;

   /* make sure Gno is running */
   if (needsgno()==0) {
      fprintf(stderr,"Requires Gno/ME\n");
      return 1;
   }

#ifdef STACK_CHECK
   begin_stack_check();
#endif

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

#ifdef STACK_CHECK
      fprintf(stderr,"stack usage: %d bytes\n",end_stack_check());
#endif

   if ((matches1>=0) && (matches2>=0) && (matches3>=0) && i>0) return 0;
   return 1;
}
