/*

parsing code for 'edit'

1.2(jb) 10/27/91
    Added support for a '$editor' variable.  edit now checks this var so
    users can select different editors without copying files around.
1.1(jb) 10/9/91
    Removed a lot of yucky code, added error checking to make sure there
    is one and only one filename specified before 'edit' does anything.
1.0(jb)       Ick! Nasty! Unnecessary code everywhere! Ick!

*/

#pragma stacksize 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <types.h>
#include <gsos.h>
#include <shell.h>
#include <orca.h>
#include <stddef.h>
#include <texttool.h>
#include <gno/gno.h>
#include <sys/wait.h>

char *invoke;

doError(char *err)
{
   printf("%s: %s\n",invoke,err);
    exit(2);
}

char sfile[65], dfile[65], parms[256], istring[256];
Get_LInfoPB gl;

ExpandPathRecGS ep;

main (int argc,char *argv[])
{
int pid,wpid;
union wait status;
GSString255Ptr inPath;
ResultBuf255Ptr outPath;
int i;
char *editname, *editinvoke, *nm, *spc;

   invoke = argv[0];
   if (argc < 2) doError("no filename specified");
   else if (argc > 2) doError("only one filename allowed");

   sfile[1] = 0;
   strcat(sfile+1,argv[1]);
   sfile[0] = strlen(sfile+1);

   dfile[0] = parms[0] = istring[0] = 0;
   gl.pflags = 0x8000000l;
   gl.mflags = 0l;
   gl.kflag = 0;
   gl.sfile = sfile;
   gl.dfile = dfile;
   gl.parms = parms;
   gl.istring = istring;

   gl.merr = 8; gl.merrf = 0;
   gl.org = 0l;

   inPath = malloc((size_t) (sfile[0] + 2));
   inPath->length = sfile[0];
   memcpy(inPath->text,sfile+1,(size_t) sfile[0]);
   outPath = malloc(sizeof(ResultBuf255));
   outPath->bufSize = 255;
   ep.pCount = 3;
   ep.inputPath = inPath;
   ep.outputPath = outPath;
   ep.flags = 0;
   ExpandPathGS(&ep);
   if (toolerror()) { printf("edit: error %04X\n",toolerror());  exit(-1); }
   sfile[0] = outPath->bufString.length;
   memcpy(sfile+1,outPath->bufString.text,sfile[0]);
   free(inPath);
   free(outPath);

   /* hmm.  These could well be considered bugs in Rose */
   for (i = 1; i <= sfile[0]; i++) {
      if (sfile[i] == ':') sfile[i] = '/';
   }
   SET_LINFO(&gl);
   editname = getenv("editor");
   if (editname == NULL) editinvoke = editname="4/editor";
   else {
       nm = malloc(strlen(editname)+1); strcpy(nm,editname);
       spc = strchr(nm, ' ');
       if (spc == NULL) { editname = editinvoke = nm; }
       else {
           editinvoke = malloc(strlen(nm)+1); strcpy(editinvoke,nm);
           *spc = 0; editname = nm;
       }
   }
   if ((pid = exec(editname,editinvoke)) >= 0) {
   waitlp2:
       wpid = wait(&status);
       if ((wpid != pid) || (status.w_stopval == WSTOPPED)) goto waitlp2;
   }
   else perror("edit: ");
   exit(status.w_retcode);
}
