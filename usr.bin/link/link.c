/*
 * $Id: link.c,v 1.1 1998/03/31 03:32:50 gdr-ftp Exp $
 *
 
parsing code for compile, cmpl, cmplg

The options are set up in the following format:

 Low                          High
 Byte 0   Byte 1   Byte 2   Byte 3
76543210 76543210 76543210 76543210
yz       qrstuvwx ijklmnop abcdefgh

kflag
   0 - do not save output
   1 - save to object filename pointed to by dfile
   2 - .root has already been made, make .a files
   3 - at least one alphabetic suffix has been used

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <types.h>
#include <unistd.h>
#include <shell.h>
#include <orca.h>
#include <texttool.h>
#include <gno/gno.h>
#include <sys/wait.h>

longword optTab[] = {
 0x80000000,
 0x40000000,
 0x20000000,
 0x10000000,
 0x08000000,
 0x04000000,
 0x02000000,
 0x01000000,
 0x00800000,
 0x00400000,
 0x00200000,
 0x00100000,
/* 0x00080000,*/ 0x00000000, /* disabled because of FASTFILE bug */
 0x00040000,
 0x00020000,
 0x00010000,
 0x00008000,
 0x00004000,
 0x00002000,
 0x00001000,
 0x00000800,
 0x00000400,
 0x00000200,
 0x00000100,
 0x00000080,
 0x00000040
};

char *invoke;

void
doError(char *err)
{
   printf("%s: %s\n",invoke,err);
   exit(2);
}

void
expandVar2dfile(char *varname,Get_LInfoPB *g)
{
char varval[80];
Get_VarPB gp;
int i,n = 1;
int l;

    gp.var_name = varname;
    gp.value = varval;
    GET_VAR(&gp);
    for (i = 1; i <= varval[0]; i++) {
        if (varval[i] == '$') {
            memcpy(g->dfile+n, g->sfile+1,g->sfile[0]);
            n+=g->sfile[0];
        }
        else if (varval[i] == '%') {
            doError("% operator in KeepName not supported");
        }
        else g->dfile[n++] = varval[i];
    }
    g->dfile[0] = n-1;
}

char sfile[65], dfile[65], parms[256], istring[256];
Get_LInfoPB gl;

int
main (int argc,char *argv[])
{
int lastind,ind = 0;
char *cmd1,*cmdline;
int pid,wpid;
#if 1
union wait wstat;
#else
unsigned wstat;
#endif
int numObjs = 0;

   __REPORT_STACK();
   
   invoke = basename(argv[0]);
   if (!strcmp(invoke,"link")) gl.lops = 2;
   else doError("invalid invokation");

   sfile[0] = sfile[1] = dfile[0] = parms[0] = istring[0] = 0;
   gl.pflags = 0x8000000l;
   gl.mflags = 0l;
   gl.kflag = 0;
   gl.sfile = sfile;
   gl.dfile = dfile;
   gl.parms = parms;
   gl.istring = istring;
   ind = 1;

/* parse option flags */
   while ((argv[ind][0] == '+') || (argv[ind][0] == '-')) {
       if (argv[ind][0] == '+')
           gl.pflags |= optTab[toupper(argv[ind][1])-'A'];
       else
           gl.mflags |= optTab[toupper(argv[ind][1])-'A'];
       ind++;
   }

/* parse object file names */
   while ((ind < argc) && (strincmp(argv[ind],"keep=",5))) {
       if (strlen(sfile+1) == 0) strcat(sfile+1,argv[ind]);
       else {
           strcat(sfile+1," ");
           strcat(sfile+1,argv[ind]);
       }
       ind++;
       numObjs++;
   }
   sfile[0] = strlen(sfile+1); /* set the length */

/* and if there's a keep field, grab it's stuff, too */
   if (ind < argc) {
     if (!strincmp(argv[ind],"keep=",5)) {
       gl.kflag = 1;
       dfile[0] = strlen(argv[ind]+5);
       strcpy(dfile+1,argv[ind]+5);
     } else doError("illegal option");
   }
   gl.merr = 0; gl.merrf = 0;
   gl.org = 0l;

   if ((gl.dfile[0] == 0) && (numObjs == 1)) {
       expandVar2dfile("\pKeepName",&gl);
       if (gl.dfile[0]) gl.kflag = 1; /* was something expanded? */
   }

/*
   printf("source address %08lX\n",&gl);
   printf("sfile       : %p\n",gl.sfile);
   printf("dfile       : %p\n",gl.dfile);
   printf("parms       : %p\n",gl.parms);
   printf("kflag       : %d\n",gl.kflag);
   printf("istring     : %p\n",gl.istring);
   printf("mflags      : %08lX\n",gl.mflags);
   printf("pflags      : %08lX\n\n",gl.pflags);
*/

   SET_LINFO(&gl);
   if ((pid = exec("16/linker","linker")) >= 0) {
   waitlp2:
       wpid = wait((union wait *) &wstat);
#if 1
	/*
	 * The GNO v2.0.4 one used the other block in this #ifdef;
	 * was this a compiler problem? -- gdr
	 */
       if ((wpid != pid) || WIFSTOPPED(wstat)) {
/*	  ((wpid != pid) || (wstat.w_S.w_Stopval == WSTOPPED)) { */
		goto waitlp2;
       }
#else
       if ((wpid != pid) ||
         ((wstat&0xFF) == WSTOPPED)) goto waitlp2;
#endif
   }
   GET_LINFO(&gl);
   exit(gl.merrf);
}
