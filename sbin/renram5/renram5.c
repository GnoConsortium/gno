/*
 * renram5
 *
 * This program is intended to be launched during boot time.  It
 * renames the volume /RAM5 to /tmp if /tmp does not already exist.
 *
 * It can also be invoked as a shell command (in which it should
 * be changed to an exec file rather than a s16 file).  As a shell
 * command, its usage is:
 *      renram5 [-d] [ oldname [newname]]
 *
 * If <oldname> is not specified, it defaults to "/RAM5".  If <newname> is
 * not specified, it defaults to "/tmp".  The -d flag enables debugging
 * output.
 *
 * You probably need GNO/ME libraries in order to link this program.
 *
 * Written by Devin Reade <gdr@gno.org> January 1996.
 * This program is placed in the public domain.
 *
 * $Id: renram5.c,v 1.4 1999/07/03 14:39:19 gdr-ftp Exp $
 *
 * This file is formatted with tab tops every 8 columns.
 */

#define __USE_DYNAMIC_GSSTRING__	/* dynamic GS/OS strings */

#include <types.h>
#include <gsos.h>
#include <orca.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef __GNO__
#include <gno/gno.h>
#endif

#ifndef P_tmpdir
#define P_tmpdir       "/tmp"
#endif

#define OLD_TMP  "/RAM5"
#define NEW_TMP  P_tmpdir

void usage(char *progname) {
  printf("Usage: %s [-d] [oldvolume [newvolume]]\n",progname);
  printf("\toldvolume defaults to %s\n",OLD_TMP);
  printf("\tnewvolume defaults to %s\n",NEW_TMP);
  printf("\t-d\tproduce debug information\n\n");
  printf("This program renames volumes.  It was intended to rename\n");
  printf("%s at boot time.  As a side effect, it can also rename files.\n\n",
	 OLD_TMP);
  printf("Version 1.0 by Devin Reade <gdr@gno.org>\n");
  printf("This program is in the public domain.\n");
  exit(1);
}

int main (int argc, char **argv) {
  DevNumRecGS devrec;
  ChangePathRecGS pathrec;
  char *file1, *file2;
  int i, filecount, debug;

#ifdef __GNO__
  __REPORT_STACK();
#endif  
  filecount=0;
  debug=0;
  
  /* parse the command line, if any */
  if (argc > 1) {
    for (i=1; i<argc; i++) {
      if (!strcmp(argv[i],"-d")) {
	debug++;
      } else if ((argv[i][0] == '-') || (filecount > 2)) {
	usage(argv[0]);
      } else if (filecount==0) {
	file1 = argv[i];
	filecount++;
      } else if (filecount==1) {
	file2 = argv[i];
	filecount++;
      } else assert(0);
    }
  }
  switch (filecount) {
  case 0:
    file1 = OLD_TMP;
    file2 = NEW_TMP;
    break;
  case 1:
    file2 = NEW_TMP;
    break;
  case 2:
    break;
  default:
    assert(0);
  }
  
  assert(file1);
  assert(file2);
  
  /*
   * see if file2 is already around
   */
  
  devrec.pCount = 2;
  if ((devrec.devName = __C2GSMALLOC(file2)) == NULL) {
    perror("couldn't duplicate destination volume name");
    exit(1);
  }
  GetDevNumberGS(&devrec);
  i=toolerror();
  switch (i) {
  case 0:
    if (debug) {
      printf("volume %s already exists on device %d\n",file2,
	     devrec.devNum);
    }
    exit(1);
  case devNotFound:
  case volNotFound:
    /* this is what we're normally expecting */
    break;
  default:
    fprintf(stderr,"couldn't get %s device number: %s\n",file2,
            strerror(_mapErr(i)));
    exit(1);
  }

  /*
   * rename the volume
   */
  
  pathrec.pCount      = 3;
  pathrec.pathname    = __C2GSMALLOC(file1);
  pathrec.newPathname = __C2GSMALLOC(file2);
  pathrec.flags       = 0;
  if (!pathrec.pathname || !pathrec.newPathname) {
    perror("couldn't duplicate volume names");
    exit(1);
  }
  ChangePathGS(&pathrec);
  i=toolerror();
  switch (i) {
  case 0:
    if (debug) printf("device renamed\n");
    break;
  case pathNotFound:
  case volNotFound:
    if (debug) printf("device not renamed: %s\n",strerror(_mapErr(i)));
    break;
  default:
    fprintf(stderr,"couldn't rename %s to %s: %s\n",file1, file2,
            strerror(_mapErr(i)));
    exit(1);
  }
  
  return 0;
}   
