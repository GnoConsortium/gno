/*
 * comp.c
 *
 * parsing code for compile, cmpl
 *
 * 'cmplg' is not supported
 *
 * Copyright 1991-1998 Procyon, Inc.
 *
 *	March 12, 1992 - switched from strcmp to stricmp on invoke name check
 *
 * $Id: compile.c,v 1.1 1998/02/15 00:05:28 gdr-ftp Exp $
 */

/*
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
#include <shell.h>
#include <gsos.h>
#include <orca.h>
#include <texttool.h>
#include <unistd.h>
#include <gno/gno.h>
#include <sys/wait.h>

#define PATH_LINKER "16/linker"
#define PATH_EDITOR "15/editor"

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
 0x00000000,/* 0x00080000,*/ /* disable +m option because of bug! */
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

char *sources[40];
int numSources;

typedef struct langInfo {
    word number;
    char *name;
    char *info;
} langInfo;

langInfo langTable[] = {
   {3, "asm65816", NULL},
   {4, "IBASIC", NULL},
   {5, "pascal", NULL},
   {7, "smallc", NULL},
   {8, "cc", NULL},
   {9, "link", NULL},
   {10, "apwc", NULL},
   {11, "pascal", NULL},
   //{16, "modula2", NULL},
   {21, "rez", NULL},
   {260, "basic", NULL},
   {265, "linker", NULL},
   {272, "modula2", NULL},
   {-1,"",NULL}};

char *invoke;

void
doError(char *err)
{
    printf("%s: %s\n",invoke,err);
    exit(2);
}

int parseSp(int i, char *s)
{
    while ((s[i] != ' ') && (s[i])) { i++; }
    if (s[i]) return ++i;
    else return i;
}

int parsecp(int i, char *s)
{
    while ((s[i] != ')') && (s[i])) { i++; }
    return i;
}

char *expandFName(char *name)
{
    char *p,*q;
    Expand_DevicesPB ed;

    p = malloc(256l);
    memcpy(p,name,name[0]+1);
    ed.pathname = p;
    EXPAND_DEVICES(&ed);
    q = malloc(p[0]+1);
    memcpy(q,p,p[0]+1);
    free(p);
    return q;
}

void
expandVar2dfile(char *varname, Get_LInfoPB *g)
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
            for (l = g->sfile[0]; l > 0; l--) {
                if (g->sfile[l] == '.') {
                    memcpy(g->dfile+n,g->sfile+1,(size_t)l-1);
                    n+=l-1;
                    break;
                }
            }
        }
        else if (varval[i] == '%') {
            doError("% operator in KeepName not supported");
        }
        else g->dfile[n++] = varval[i];
    }
    g->dfile[0] = n-1;
}

#ifdef __STACK_CHECK__
static void
printStack (void) {
	fprintf(stderr, "stack usage: %d bytes\n", _endStackCheck());
}
#endif

char		sfile[65], dfile[65], parms[256], istring[256];
char		linksfile[256];
Get_LInfoPB	gl;
GSString255	pathGS;

int
main (int argc,char *argv[])
{
   static char invokeCmd[40];
   int lastind, ind=0, langind;
   char *cmd1, *cmdline, *temps;
   int pid, wpid, index, sind;
   FileInfoRecGS fi;
   int err;
   GSString255Ptr expPathGS;
   union wait status;

#ifdef __STACK_CHECK__
   _beginStackCheck();
   atexit(printStack);
#endif

   invoke = basename(argv[0]);
   if ((strcasecmp(invoke, "compile") == 0) ||
       (strcasecmp(invoke, "assemble") == 0)) {
   	gl.lops = 1;
   } else if ((strcasecmp(invoke, "cmpl") == 0) ||
   	      (strcasecmp(invoke, "asml") == 0)) {
   	gl.lops = 3;
   } else {
   	doError("invalid invocation");
   }

   cmd1 = commandline();
   cmdline = malloc(strlen(cmd1)+1);
   strcpy(cmdline,cmd1);

   sfile[0] = dfile[0] = parms[0] = istring[0] = 0;
   gl.pflags = 0x00000000l;
   gl.mflags = 0l;
   gl.kflag = 0;
   gl.sfile = sfile;
   gl.dfile = dfile;
   gl.parms = parms;
   gl.istring = istring;
   numSources = 0;
   linksfile[1] = 0;

   ind = parseSp(ind,cmdline);
   while ((cmdline[ind] == '+') || (cmdline[ind] == '-')) {
       if (cmdline[ind] == '+')
           gl.pflags |= optTab[toupper(cmdline[ind+1])-'A'];
       else
           gl.mflags |= optTab[toupper(cmdline[ind+1])-'A'];
       ind = parseSp(ind,cmdline);
   }

   lastind = ind;
/*   ind = parseSp(ind,cmdline);
   sfile[0]=ind-lastind;
   if (cmdline[ind]) sfile[0]--;
   strncpy(sfile+1,(cmdline+lastind),sfile[0]); */

    while (cmdline[ind]) {
	if (!strncasecmp(cmdline+ind,"keep=",5)) {
            gl.kflag = 1;
            lastind = ind;
            ind = parseSp(ind,cmdline);
            dfile[0] = (ind-lastind)-5;
            if (cmdline[ind]) dfile[0]--;
            strncpy(dfile+1,(cmdline+lastind+5),dfile[0]);
            continue;
	}
	if (!strncasecmp(cmdline+ind,"names=(",7)) {
            lastind = ind;
            ind = parsecp(ind,cmdline);
            parms[0] = (ind-lastind)-7;
            strncpy(parms+1,(cmdline+lastind+7),parms[0]);
            ind = parseSp(ind,cmdline);
            continue;
	}
	langind = 0;
	while (langTable[langind].number != -1) {
	int l = strlen(langTable[langind].name);

            if ((!strncasecmp(cmdline+ind,langTable[langind].name,l)) &&
        	(*(cmdline+ind+l) == '=')) {
        	lastind = ind;
        	ind = parsecp(ind,cmdline);
               
        	langTable[langind].info = malloc(ind-lastind-l+2);
        	langTable[langind].info[0] = (ind-lastind-l-2);
        	strncpy(langTable[langind].info+1,(cmdline+lastind+l+2),
                    langTable[langind].info[0]);
        	ind = parseSp(ind,cmdline);
        	goto loopit;
            }
            langind++;
	}
	lastind = ind;
	ind = parseSp(ind,cmdline);
	temps = sources[numSources] = malloc(ind-lastind+2);
	strncpy(sources[numSources]+1,(cmdline+lastind),(ind-lastind));
	sources[numSources][0] = ind-lastind;
	if (cmdline[ind]) sources[numSources][0]--;
	sources[numSources]=expandFName(sources[numSources]);
	free(temps);
	numSources++;
loopit: ;
    }
    if (numSources == 0) { fprintf(stderr,"No source files specified.\n");
	exit(1); }
    gl.merr = 0; gl.merrf = 0;
    gl.org = 0l;

    for (sind = 0; sind < numSources; sind ++) {
	memcpy(sfile,sources[sind],sources[sind][0]+1);
  /* if no keepname is specified on the command line,
     check the variable "KeepName" for one */

	if (gl.dfile[0] == 0) {
            expandVar2dfile("\pKeepName",&gl);
            if (gl.dfile[0]) gl.kflag = 1; /* was something expanded? */
	} 

	pathGS.length = sources[sind][0];
	memcpy(pathGS.text,sources[sind]+1,pathGS.length);

	dfile[dfile[0]+1] = 0;
	strcat(linksfile+1,dfile+1);
	strcat(linksfile+1," ");

	fi.pCount = 4;
	fi.pathname = &pathGS;
	GetFileInfoGS(&fi);
	if (err = toolerror()) {
            printf("%s:",invoke);
            ERROR(&err);
            exit(err);
	}
	index = 0;
	while (langTable[index].number != -1) {
            if (((word)fi.auxType) == langTable[index].number) break;
            index++;
	}
	if (langTable[index].number == -1) doError("Unsupported language type");
	if (langTable[index].info != NULL)
          memcpy(gl.istring,langTable[index].info,langTable[index].info[0]+1);
	else gl.istring[0] = 0; /* set istring to null */
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

	sprintf(invokeCmd,"16/%s",langTable[index].name);
	if ((pid = exec(invokeCmd,invokeCmd)) >= 0) {
	waitlp1:
            wpid = wait(&status);
            if ((wpid != pid) || (status.w_stopval == WSTOPPED)) goto waitlp1;
        }
        GET_LINFO(&gl);
        if ((gl.merrf > gl.merr) && (langTable[index].number != 9) &&
         	(langTable[index].number != 265) && (gl.pflags & 0x08000000l)) {
           printf("%s:[%ld] %p\n",langTable[index].name,gl.org,gl.parms);
	   if ((pid = exec(PATH_EDITOR,PATH_EDITOR)) >= 0) {
	    waitlp9:
	       wpid = wait(&status);
	       if ((wpid != pid) || (status.w_stopval == WSTOPPED)) goto waitlp9;
	   }
           exit(gl.merrf);
	}
   }
   /*gl.kflag = 1;*/

   /*linksfile[0] = strlen(linksfile+1);
   memcpy(sfile,linksfile,linksfile[0]+1);*/

   SET_LINFO(&gl);
   if (gl.lops & 2)
   if ((pid = exec(PATH_LINKER,PATH_LINKER)) >= 0) {
    waitlp2:
       wpid = wait(&status);
       if ((wpid != pid) || (status.w_stopval == WSTOPPED)) goto waitlp2;
   }
   GET_LINFO(&gl);
   exit(gl.merrf);
}
