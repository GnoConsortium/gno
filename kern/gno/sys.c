/*	$Id: sys.c,v 1.1 1998/02/02 08:19:05 taubert Exp $ */

#pragma optimize 79
segment "KERN2     ";

/*

       sys.c

       GS/IX Kernel routines for process control

       Copyright 1991-1998, Procyon, Inc.
       Jawaid Bazyar and Derek Taubert
*/

#include "conf.h"
#include "proc.h"
#include "kernel.h"
#include "gno.h"
#include "kvm.h"
#include "sys.h"
#include "/lang/orca/libraries/orcacdefs/stdio.h"
#include "/lang/orca/libraries/orcacdefs/string.h"
#include "/lang/orca/libraries/orcacdefs/stdlib.h"
#include <loader.h>
#include <memory.h>
#include <gsos.h>
#include <shell.h>
#include <texttool.h>
#include <misctool.h>
#include <sys/errno.h>
#include <sys/times.h>

extern kernelStructPtr kp;

/* signal context record */
typedef struct ctxt {
   word ctx_A;
   word ctx_X;
   word ctx_Y;
   word ctx_state;
   word ctx_D;
   byte ctx_B;
   byte ctx_B1;
   word ctx_P;
   word ctx_S1;
   longword ctx_PC;
} ctxt, *ctxtPtr;

struct GSString1K {
   Word length; /* Number of Chars in text field  */
   char text[1024];
} ;
typedef struct GSString1K GSString1K, *GSString1KPtr;

struct ResultBuf1K {
   Word bufSize;
   GSString1K bufString;
} ;
typedef struct ResultBuf1K ResultBuf1K, *ResultBuf1KPtr;

/*
   this is the info kept per-process on what sorts of shit the process
   expects for I/O.  This is similar to a special case of user.h fd table
   including only the stdin/stdout fds (0-2).
*/

typedef struct ttyinf {
   word InANDMask;
   word InORMask;
   word OutANDMask;
   word OutORMask;
   word ErrANDMask;
   word ErrORMask;
   word InDeviceType;
   word OutDeviceType;
   word ErrDeviceType;
   longword InSlot;
   longword OutSlot;
   longword ErrSlot;
   word dummy;
} ttyinf;

#define NPGRP 32

pgrp pgrpInfo[NPGRP];
typedef struct ttyInfo {
    word pgrp;
} ttyInfo;

extern ttyStruct;
ttyInfo *ttys = (ttyInfo *) &ttyStruct;

#define isbadpgrp(p) ((p < 0) || (p >= NPGRP))

static word hiWord(longword w)
{
word h;

  asm {
    lda w+2
    sta h
  }
  return h;
}

static word loWord(longword w)
{
word l;

  asm {
    lda w
    sta l
  }
  return l;
}

void *kmalloc(size_t size)
{
handle h;
word p_uid;
void *x;

    /* $$$ p_uid = kp->procTable[Kgetpid()].userID; */
    p_uid = PROC->userID;
    h = NewHandle(size, p_uid, 0xC008, NULL);

    if (toolerror()) {
	fprintf(stderr, "kmalloc:%04X\n",toolerror());
	return NULL;
    }
    x = *h;
    return x;
}

void *pmalloc(size_t size, word p_uid)
{
handle h;
void *x;

    h = NewHandle(size, p_uid, 0xC008, NULL);

    if (toolerror()) {
	fprintf(stderr, "pmalloc:%04X\n",toolerror());
	return NULL;
    }
    x = *h;
    return x;
}

int kfree(void *mem)
{
handle h;

    if (h = (FindHandle(mem) == NULL)) return SYSERR;
    DisposeHandle(h);
    return (OK);
}

static int findEmptyProc(void)
{
int i;
   for (i = 0; i < NPROC; i++)
       if (kp->procTable[i].processState == procUNUSED) return i;
   return -1;
}

#if 0
int isbadpid( int pid )
{
int mpid;

    mpid = mapPID(pid);
    if (mpid == -1) return SYSERR;
    else return SYSOK;
}
#endif

static int createProc(word stack, word dp, longword process)
{
int p;

    disableps();
 /*   printf("creating process stack=%04X, dp=%04X, addr=%08lX\n",
        stack, dp, process); */
    p = findEmptyProc();
    if (p == -1) { enableps(); return SYSERR; }
    kp->procTable[p].processState = procSUSPENDED;
    kp->procTable[p].parentpid = Kgetpid();
    kp->procTable[p].irq_S = stack;
    kp->procTable[p].irq_D = dp;
    kp->procTable[p].irq_PC = (word) process;
    kp->procTable[p].irq_B =
      kp->procTable[p].irq_K = (process >> 16);
    kp->procTable[p].irq_P = 4; /* interrupts must be OFF you putz */
    /* assign child same TTY as parent */
    kp->procTable[p].ttyID = PROC->ttyID;
    	/* $$$ kp->procTable[Kgetpid()].ttyID; */
    enableps();
    return p;
}

#if 0
#define BUSY_FLAG ((byte *)0xE100FFl)

int _ready(int pid, int resch)
{
int mpid;
unsigned x;

 	mpid = mapPID(pid);
	if (mpid == -1) return SYSERR;
	disableps();
	if (kp->procTable[mpid].processState != procUNUSED)
		kp->procTable[mpid].processState = procREADY;
	if (resch) {
            asm {
                php
                sei
            }
            x = *BUSY_FLAG;
            *BUSY_FLAG = 0;
            _resched();
            *BUSY_FLAG = x;
            asm {
	    	plp
	    }
        }
	enableps();
	return OK;
}
#endif

/*
 *	Allocates a new file descriptor in this process.  If the fd table is
 *	full, resizes the table and proceeds with the allocation.
 *	allocFD does NOT adjust the fdCount field, as it's possible that
 *	we must unallocate the fd later due to an error and it saves
 *	a bit of code. (i.e. the caller must inc fdCount)
 */
fdentryPtr allocFD(int *fdn)
{
fdtablePtr ft;
int i,j;
unsigned s;

    /* $$$ ft = kp->procTable[Kgetpid()].openFiles; */
    ft = PROC->openFiles;
    j = 0; /* change this when tables are dynamically allocated */
    disableps();
retry:
    while (j < ft->fdTableSize) {
      if (ft->fds[j].refNum == 0) {
        ft->fds[j].refNum = -1; /* just to make this item look allocated */
        enableps();
        if (fdn != NULL) *fdn = j+1;
            return &(ft->fds[j]);
      } else j++;
    }
    ft->fdTableSize += 32;
    i = ft->fdTableSize;
    ft = realloc(ft,sizeof(fdtable) + sizeof(fdentry)*(i-1));
    /* zero out the new entries */
    memset(&ft->fds[i-32],0,sizeof(fdentry)*32);
    PROC->openFiles = ft;
    goto retry;
}

struct rc {
   int rn;
   int rt;
   int cn;
};

#ifdef NOTDEFINED
void printFDS(fdtablePtr f)
{
int i;
struct rc *r;
extern struct rc *FINDREFNUM(int,int);

    fputc('[',stderr);
    for (i = 0; i < 32; i++) {
        if (f->fds[i].refNum) {
            fprintf(stderr,"(%d ",i);
            switch (f->fds[i].refType) {
                case rtGSOS: fprintf(stderr,"GSOS "); break;
                case rtPIPE: fprintf(stderr,"PIPE "); break;
                case rtTTY : fprintf(stderr,"TTY  "); break;
                default    : fprintf(stderr,"%d ",f->fds[i].refType); 
            }
            fprintf(stderr,"%d/%d ", f->fds[i].refNum,f->fds[i].refLevel);
            r = FINDREFNUM(f->fds[i].refType,f->fds[i].refNum);
            if (r == NULL) fprintf(stderr,"!!)");
            else fprintf(stderr,"%d)",r->cn);
        }
    }
    fputc(']',stderr); fputc('\n',stderr);
}
#endif

#pragma databank 1
void endproc(void)
{
/*
 * printfs in here won't work with quiting orca/c progs because ~C_SHUTDOWN
 * closes ALL open file descriptors
 */

/*  printf("end of process %d\n",Kgetpid());  */
 /* $$$   kp->procTable[Kgetpid()].flags |= FL_NORMTERM; */
    PROC->flags |= FL_NORMTERM;
 /*   kill(kp->procTable[Kgetpid()].flpid,9); */
    Kkill(&errno, 9, PROC->flpid);
    /* not reached */
    PANIC("ENDPROC KILL OVERRUN");
}

int CommonQuit(char *pname, int flag)
{
/*struct pentry *p = &kp->procTable[Kgetpid()];*/
char *pcopy;
quitStack *qs;
GSString255Ptr gp;

    enableps();
    if (flag & 0x4000) PROC->flags |= FL_RESTART;
    if (pname && *pname) {
        if (flag & 0x8000) {
            gp = (GSString255Ptr) LGetPathname2(PROC->userID,1);
            qs = malloc(gp->length+5);
            qs->next = PROC->returnStack;
            memcpy(qs->data,gp->text,gp->length);
            qs->data[gp->length] = 0;
            PROC->returnStack = qs;
        }
        pcopy = kmalloc(strlen(pname)+1);
        strcpy(pcopy,pname); nfree(pname);
        Kexecve(&errno,pcopy,pcopy);
    }
    else {
        if ((qs = PROC->returnStack) != NULL) {
            pcopy = kmalloc(strlen(qs->data)+1);
            strcpy(pcopy,qs->data);
            PROC->returnStack = qs->next; nfree(qs);
            Kexecve(&errno,pcopy,pcopy);
        } else Kkill(&errno, 9, PROC->flpid);
    }
    disableps();
    return 0x27;
}

#pragma toolparms 1
/* subr is the address to begin the process at */

pascal int KERNSetGNOQuitRec(word pCount,GSString255Ptr pathname, word flags, int *ERRNO)
{
extern QuitRecGS quitParms;
    quitParms.pCount = pCount;
    quitParms.pathname = pathname;
    quitParms.flags = flags;
}

int KERNgetpid()
/* get the process id of currently executing process */
{
 /*$$$ return( kp->procTable[kp->truepid].flpid ); */
    return (PROC->flpid);
}

int KERNgetppid(int *ERRNO)
/* get the pid of the process' parent */
{
    return (kp->procTable[PROC->parentpid].flpid);
}

int KERNgetpgrp(int *ERRNO, int pid)
/* get the pgrp field for a specified process */
{
int mpid;
  mpid = mapPID(pid);
  if (mpid == -1) { *ERRNO = ESRCH; return -1; }
  return( kp->procTable[mpid].pgrp );
}

int KERNsetpgrp(int *ERRNO, int pgrp, int pid)
{
int mpid;
int pp;

    mpid = mapPID(pid);
    if (mpid == -1) { *ERRNO = ESRCH; return -1; }
    if (isbadpgrp(pgrp)) { *ERRNO = ESRCH; return -1; }
   if (kp->procTable[mpid].pgrp != pgrp)
   {
       if (pgrp != 0) pgrpInfo[pgrp-2].pgrpref++;
       if ((pp = kp->procTable[mpid].pgrp) != 0)
          pgrpInfo[pp-2].pgrpref--;
   }
   return 0;
}

int KERNgetuid(int *ERRNO)
{
    return (PROC->p_uid);
    /* $$$ return (kp->procTable[Kgetpid()].p_uid); */
}

int KERNgeteuid(int *ERRNO)
{
    return (PROC->p_euid);
    /* $$$ return (kp->procTable[Kgetpid()].p_euid); */
}

int KERNgetgid(int *ERRNO)
{
    return (PROC->p_gid);
    /* $$$ return (kp->procTable[Kgetpid()].p_gid); */
}

int KERNgetegid(int *ERRNO)
{
    return (PROC->p_egid);
    /* $$$ return (kp->procTable[Kgetpid()].p_egid); */
}

int KERNsetreuid(int *ERRNO, int ruid, int euid)
{
    if (ruid == -1) ruid = PROC->p_uid;
    if (euid == -1) euid = PROC->p_euid;
    if ((PROC->p_uid != 0) &&
    	(((euid != PROC->p_uid) && (euid != PROC->p_euid))
	|| ((ruid != PROC->p_euid) && (ruid != PROC->p_uid)))) {
	*ERRNO = EPERM;
	return -1;
    }
    PROC->p_uid = ruid;
    PROC->p_euid = euid;
    return 0;
}

int KERNsetuid(int *ERRNO, int uid)
{
/* $$$ struct pentry *p;
p = &(kp->procTable[Kgetpid()]);  */

    if ((PROC->p_uid == 0) || (uid == PROC->p_uid) || (uid == PROC->p_euid)) {
	PROC->p_uid = uid;
        PROC->p_euid = uid;
	return 0;
    }
    *ERRNO = EPERM;
    return -1;
}

int KERNsetregid(int *ERRNO, int rgid, int egid)
{
    if (rgid == -1) rgid = PROC->p_gid;
    if (egid == -1) egid = PROC->p_egid;
    if ((PROC->p_uid != 0) &&
    	(((egid != PROC->p_gid) && (egid != PROC->p_egid))
	|| ((rgid != PROC->p_egid) && (rgid != PROC->p_gid)))) {
	*ERRNO = EPERM;
	return -1;
    }
    PROC->p_gid = rgid;
    PROC->p_egid = egid;
    return 0;
}

int KERNsetgid(int *ERRNO, int gid)
{
/* $$$ struct pentry *p;
p = &(kp->procTable[Kgetpid()]);  */

    if ((PROC->p_uid == 0) || (gid == PROC->p_gid) || (gid == PROC->p_egid)) {
	PROC->p_gid = gid;
        PROC->p_egid = gid;
	return 0;
    }
    *ERRNO = EPERM;
    return -1;
}

int KERNtimes(int *ERRNO, struct tms *buffer)
{
 /* $$$   buffer->tms_utime = kp->procTable[Kgetpid()].ticks;
    buffer->tms_cutime = kp->procTable[Kgetpid()].childTicks;
    buffer->tms_stime = buffer->tms_cstime = 0l;  */

    buffer->tms_utime = PROC->ticks;
    buffer->tms_cutime = PROC->childTicks;
	/* a->i1 = a->i2 = b BROKEN in C 2.0.3 */
#if 0
    buffer->tms_stime = buffer->tms_cstime = 0l;
#else
    buffer->tms_stime = 0l;	buffer->tms_cstime = 0l;
#endif
    
    return 0;
}

#pragma toolparms 0
char *a_strncpy_max(char *s, word max_len)
{
word l;
char *x;

    if (s == NULL) return NULL;
    while ((l < max_len) && (s[l] != 0))
	l++;
    x = malloc(l+1);
    strncpy(x,s,l);
    x[l] = 0;
    return x;
}
    
int commonFork(void (*funcptr)(), word stackSize, int prio, char *name,
         word *argv, int *ERRNO)
{
word dPageAddr, buffSize,nargs;
int newID,newPID,parentPID;
int flpid;
longword *ret;
handle fstack;
struct pentry *parent,*child;
word state;
byte statereg,slot;
int i,j,k;
extern void endproc2(void);
extern void FORKInitGlob(void);
extern void IncRefnum(int,int);
extern void incPipe(int,int);
extern int allocPID(void);

/*   printf("address = %08lX\n",subr); */
   if (kp->gsosDebug & 16) fprintf(stderr, "fork(%06lX)\n",funcptr);
   newID = GetNewID(0x1000);

   fstack = NewHandle((long) stackSize,newID | 0x0100,0xC105, NULL);
   if (toolerror()) { *ERRNO = ENOMEM; return -1; }
   dPageAddr = (word) *fstack;
   buffSize = stackSize;

   /* calculate the # of bytes of parameters */
   nargs = (*argv) * 2;
   ret = (longword *) (dPageAddr+buffSize-4-nargs);
   /* place this first, or it'll clobber some parameters */
   *ret = ((longword) endproc2)-1; /* go to endproc when process is done */

   /* if they're passing in params, copy them to the target stack */
   if (nargs)
        memcpy( (((byte *) ret)+3), argv+1, nargs);

   disableps();
   newPID = createProc(dPageAddr+buffSize-5-nargs,dPageAddr, (longword) funcptr);
   if (newPID == SYSERR) {
       *ERRNO = EAGAIN; DisposeHandle(fstack);
       enableps(); return -1;
   }
   child = &(kp->procTable[newPID]);
   parent = PROC; /* $$$ &(kp->procTable[Kgetpid()]); */

   child->parentpid = Kgetpid();
   child->irq_A = newID;
   child->irq_X =
       child->irq_Y = 0;
   child->userID = newID;
   if (name != NULL) {
       name = a_strncpy_max(name,128);
       child->args = malloc(strlen(name)+9);
       strcpy(child->args,"BYTEWRKS");
       strcat(child->args,name);
       nfree(name);
   } else child->args = NULL;

/* make sure we keep the pgrp refcount up to date */
   child->pgrp = parent->pgrp;
   if (child->pgrp != 0) pgrpInfo[child->pgrp-2].pgrpref++;

   child->siginfo = malloc(sizeof(struct sigrec));
   memcpy(child->siginfo, parent->siginfo, sizeof(struct sigrec));
   child->waitq = NULL;

   i = sizeof(fdentry)*(parent->openFiles->fdTableSize-1);
   child->openFiles = malloc(sizeof(fdtable)+i);
   memcpy(child->openFiles, parent->openFiles, sizeof(fdtable)+i);

   child->prefix = malloc(33*sizeof(GSString255Ptr));
   for (i = 0; i < 33; i++) {
       if (parent->prefix[i] != NULL) {
	   child->prefix[i] =
              malloc( ((GSString255Ptr) parent->prefix[i])->length+2 );
	   copygsstr(child->prefix[i],parent->prefix[i]);
       } else child->prefix[i] = NULL;
   }

   child->LInfo = parent->LInfo;
   child->ticks = 0l;
   child->childTicks = 0l;
   child->flags = FL_FORKED | FL_COMPLIANT;
   child->ttyID = parent->ttyID;
   child->executeHook = parent->executeHook;
   child->returnStack = NULL;
   child->p_uid = parent->p_uid;
   child->p_gid = parent->p_gid;
   child->p_euid = parent->p_euid;
   child->p_egid = parent->p_egid;
   child->SANEwap = parent->SANEwap;
   child->lastTool = parent->lastTool; /* actually the resource app number */
   asm {
       lda 0xE0C035
       sta state
       sep #0x30
       lda 0xE0C02D
       sta slot
       lda 0xE0C068
       sta statereg
       rep #0x30
   }
   child->irq_state = state;
   child->irq_SLTROM = slot;
   child->irq_STATEREG = statereg;
   child->alarmCount = 0l;
   child->t2StackPtr = 0;
   child->processState = procNEW; /* brand new! */

   parentPID = Kgetpid();
   asm {
       ldy newPID
       ldx parentPID
       jsl FORKInitGlob
   }

/* make sure the system knows about all of our new copies of those 
   file descriptors we inherited */

   i = child->openFiles->fdCount; k = 0;
   while (i) {
	if (j = child->openFiles->fds[k].refNum) {
            if (child->openFiles->fds[k].refType == rtPIPE)
            	incPipe(child->openFiles->fds[k].refFlags,j);
            IncRefnum(child->openFiles->fds[k].refType,j);
            i--;
	}
        k++;
        if (k > child->openFiles->fdTableSize)
       	    PANIC("BAD FILE DESCRIPTOR TABLE");
   }

   child->flpid = allocPID();
   kp->numProcs++;
   enableps();
   return child->flpid;
}

#pragma toolparms 1

int KERNfork(int *ERRNO,void *subr)
{
word nargs = 0;

    return commonFork((void *)subr, 1024, 0, NULL, &nargs, ERRNO);
}

pascal int KERNfork2(void (*funcptr)(), word stackSize, int prio,
	char *name, word *argv, int *ERRNO)
{
    return commonFork(funcptr, stackSize, prio, name, argv, ERRNO);
}


typedef struct GetUserPathRec {
    int pCount;
    int fileSysID;
    int commandNum;
    GSString255Ptr prefixPtr;
} GetUserPathRec;

/*
 * This MUST be in a function by itself so the PROC-> dereference will have
 * an allocated DP to use.
 */
static void kill_self(void)
{
	Kkill(&errno, 9, PROC->flpid);
}

int KERNexecve(int *ERRNO, char *cmdline, char *filename)
{
union {
    InitialLoadOutputRec il;
    RestartOutRec r;
} il_out;
#define il_rec il_out.il
#define r_rec  il_out.r
int e;
word newID;
GSString255Ptr pfn;
ResultBuf1KPtr resBuf;
int ind;
longword *ret;
handle fstack;
int *optionList;
char **ourPFhand;
char *args,*argscopy;
int i,j;
struct pentry *p;
ExpandPathRecGS ep;
PrefixRecGS sp;
FileInfoRecGS fi;
int ssf = 0,restart,force_norest = 0;
word oldUserID, oldFlags, newStack;
extern ctxt ctxtstuff;
extern GSString255Ptr __C2GSMALLOC(char *);
extern void endproc2(void);
extern void execveHook(void);
extern void disableBuf(void);
extern void enableBuf(void);

/* always disableps() when screwing with process tables */
   disableps();
   if (kp->gsosDebug & 16) fprintf(stderr,"execve(%s,%s)\n",filename,cmdline);
  
   p = PROC; /* $$$ &(kp->procTable[Kgetpid()]);*//* aaaaaaaaaaarrrgghhh!!!!! */

/* Don't do anything with the file tables- we simply pass them on to the
   new process */

/* turn the input path into a GSString255 */
   pfn = __C2GSMALLOC(filename);

/* do an ExpandPath on the input path */
   resBuf = malloc(sizeof(ResultBuf1K));
   ep.pCount = 2;
   ep.inputPath = pfn;
   resBuf->bufSize = 1024;
   ep.outputPath = (ResultBuf255Ptr) resBuf;
   ExpandPathGS(&ep);

   restart = 1;
   newID = GetUserID2((Pointer)&resBuf->bufString);
   if (toolerror() == 0x1101) {
     /* allocate a new UserID for the program */
     newID = GetNewID(0x1000);
     restart = 0;
   } else {
/* if the 'newID' is being used by a process, force another copy from disk */
       for (i = 0; i < NPROC; i++) {
           if (kp->procTable[i].processState != procUNUSED)
               if (kp->procTable[i].userID == newID) {
                   newID = GetNewID(0x1000);
                   restart = 0; force_norest = 1;
                   break;
               }
       }
   }

    if (restart) {
	r_rec = Restart(newID);
    } else {
	il_rec = InitialLoad2(newID, (Pointer)&resBuf->bufString, 1, 1);
    }
    if ((e = toolerror())) {
	switch(e) {
	  case volNotFound:
	  case pathNotFound:
	  case fileNotFound:
	    *ERRNO = ENOENT;
	    break;
	  default:
	    *ERRNO = EIO;
	    break;
	}
	nfree(resBuf);
	enableps();
	return -1;
    }

/* get info about the executable so we know how to set up the environment */
   optionList = malloc(48l); /* should be large enough for everything */
   fi.pCount = 8;
   fi.pathname = (GSString255Ptr) &(resBuf->bufString);
   fi.optionList = (void *) optionList;
   optionList[0] = 48;
   GetFileInfoGS(&fi);

/* if the executable had no defined stack segment, create 1K by default */
   if (il_rec.buffSize == 0) {  /* no stack/dp segment- create one */
       fstack = NewHandle(4096l,newID,0xC015, NULL);
       il_rec.dPageAddr = (word) *fstack;
       il_rec.buffSize = 4096;
   }

/*  S16s (and others) get no command line arguments, but we should put
    their name in the process table anyway */

   if (cmdline != NULL) {
       size_t len = strlen(cmdline)+11;
         /* build the command line by tacking the shellid() onto the real
            line */
       args = (char *) malloc(len);
       strcpy(args,"BYTEWRKS");
       strcat(args, cmdline);
       args[((word)(len-1))] = 0; /* Orca 2.0 does this for compat */
       argscopy = pmalloc(len,newID);
       memcpy(argscopy,args,len);
   } else args = argscopy = NULL;

/* tricky- shove the address of our generic RTL-exiting shutdown routine
   onto the process stack so we can always keep track of processes */
   ret = (longword *) (il_rec.dPageAddr+il_rec.buffSize-4);
   *ret = ((longword) endproc2)-1; /* go to endproc when process is done */

/* set up the registers to point to the command line/shell id combo */
   if (p->args) nfree(p->args); /* free the args string of the old executable */
   p->args = args;
   if (fi.fileType != 0xB5) argscopy = NULL;
   ctxtstuff.ctx_A = newID;
   ctxtstuff.ctx_X = hiWord((longword) argscopy);
   ctxtstuff.ctx_Y = loWord((longword) argscopy);
   newStack = ctxtstuff.ctx_S1 = il_rec.dPageAddr+il_rec.buffSize-5;
   ctxtstuff.ctx_D = il_rec.dPageAddr;
   ctxtstuff.ctx_B = ctxtstuff.ctx_B1 = (((longword) il_rec.startAddr) >> 16);
   ctxtstuff.ctx_PC = (longword) il_rec.startAddr;

   oldUserID = p->userID;
   oldFlags  = p->flags;
   p->userID = newID;
   p->ticks  = 0l;
   p->flags  = FL_COMPLIANT;
   if (force_norest) p->flags |= FL_NORESTART;
   p->SANEwap = 0; /* reset to the 'not started up' state */
   p->lastTool = 0x401E; /* reset res app to standard */

   /* is program compliant? */
   if (fi.auxType == 0xDC00l) {
      p->flags &= (~FL_COMPLIANT); /* turn off compliance flag */
      disableBuf(); /* disablebuf now turns off the cursor */
   }

/*
 *  Set up the prefixes for the new process image
 */

    /* modify prefixes 1 and 9 to point to the directory the appl. is in */
    ourPFhand = PROC->prefix; /* $$$ kp->procTable[Kgetpid()].prefix; */
    nfree(ourPFhand[0]); /* the '@' prefix */
    nfree(ourPFhand[1+1]); nfree(ourPFhand[9+1]);

    /* set Prefixes 1 and 9 to the directory the executable is in */
    for (ind = resBuf->bufString.length-1; ind > 0; ind--)
        if (resBuf->bufString.text[ind] == ':') {
           resBuf->bufString.length = ind+1; break; /* +1 -> snag the colon */
    } /* there will always be one */

    ourPFhand[9+1] = malloc(resBuf->bufString.length+2);
    copygsstr(ourPFhand[9+1],&resBuf->bufString);
    if (optionList[2] == 0x0D) { /* app is on AppleShare volume - set '@' */
    GetUserPathRec gp;
        gp.pCount = 3; gp.fileSysID = 0x0D; gp.commandNum = 8;
        FSTSpecific(&gp);
        ourPFhand[0] = malloc(gp.prefixPtr->length+3);
        copygsstr(ourPFhand[0],gp.prefixPtr);
        ourPFhand[0][gp.prefixPtr->length+2] = ':';
        ((GSString255Ptr)ourPFhand[0])->length++;
    } else {
        ourPFhand[0] = malloc(resBuf->bufString.length+2);
        copygsstr(ourPFhand[0],&resBuf->bufString);
    }

    /* if prefix 1 would be longer than 64 chars, set it to the NULL prefix */
    if (resBuf->bufString.length > 64) resBuf->bufString.length = 0;
    ourPFhand[2] = malloc(resBuf->bufString.length+2);
    copygsstr(ourPFhand[2],&resBuf->bufString);

    nfree(pfn);
    nfree(resBuf);
    nfree(optionList);

    /* If we're launching a S16 application, copy prefix 0 to prefix 8 */
    if (fi.fileType == 0xB3) {
    	pfn = (GSString255Ptr) ourPFhand[0+1];
	pfn = malloc(pfn->length+2);
	copygsstr(pfn,ourPFhand[0+1]);
        nfree(ourPFhand[8+1]);
        ourPFhand[8+1] = (char *)pfn;
    }

/* Ignored/blocked signals remain ignored/blocked, but caught signals
   are reset to their default state */
   for (i = 0; i < 32; i++) {
       if (p->siginfo->v_signal[i] != SIG_IGN)
           p->siginfo->v_signal[i] = SIG_DFL;
   }

   SET_STOP_FLAG(&ssf);

   if (!(oldFlags & FL_COMPLIANT)) {
      enableBuf();
   }
   if ((fi.auxType & 0xDB02) != 0xDB02)
     if (oldFlags & FL_QDSTARTUP) *((byte *)0xE0C029l) &= 0x7F;
/* switch over to the new stack before we deallocate the one we're using */
   asm {
	lda newStack
	tcs
   }
   if ((fi.auxType == 0xDC00) && (p->ttyID != 3)) {
       fprintf(stderr, "Program can only run on console.\n");
       if (oldFlags & FL_FORKED) {
           DisposeAll(oldUserID);  /* process was fork()ed, don't USD */
           DeleteID(oldUserID);
       } else {
           UserShutDown(oldUserID,
            ((oldFlags & FL_RESTART) && !(oldFlags & FL_NORESTART)) ? 0x4000 : 0);
       }
       /* don't use any local variables beyond this point */
       /* STACK REPAIR MUST BE OFF TO CALL kill_self() */
       kill_self();
       /* not reached */
       PANIC("EXECVE KILL OVERRUN");
   } else {
       if (oldFlags & FL_FORKED) {
           DisposeAll(oldUserID);  /* process was fork()ed, don't USD */
           DeleteID(oldUserID);
       } else {
           UserShutDown(oldUserID,
            ((oldFlags & FL_RESTART) && !(oldFlags & FL_NORESTART)) ? 0x4000 : 0);
       }
       /* don't use any local variables beyond this point */
   }
   asm {
     jmp >execveHook
   }
}

/* open the kernel to access the process structures */

kvmt *KERNkvm_open(int *ERRNO)
{
kvmt *newk;

    newk = (kvmt *) kmalloc(sizeof(kvmt));
    if (newk == NULL) { *ERRNO = ENOMEM; return NULL;}
    Kkvmsetproc(ERRNO, newk);
    return newk;
}

SYSCALL KERNkvm_close(int *ERRNO,kvmt *k)
{
    if (kfree(k)) return SYSERR;
    return (OK);
}

struct pentry *KERNkvmgetproc(int *ERRNO, int pid, kvmt *kd)
{
int mpid;
    mpid = mapPID(pid);
    if (mpid == -1) { *ERRNO = ESRCH; return NULL; }
    memcpy(&(kd->kvm_pent), &(kp->procTable[mpid]), sizeof(struct pentry));
    kd->pid = pid;
    kd->kvm_pent.parentpid = kp->procTable[kp->procTable[mpid].parentpid].flpid;
    return &(kd->kvm_pent);
}

struct pentry *KERNkvmnextproc(int *ERRNO, kvmt *kd)
{
int i;
int mpid;

   if (kd->procIndex < NPROC)
   {
       memcpy(&(kd->kvm_pent), &(kp->procTable[kd->procIndex]),
           sizeof(struct pentry));
       kd->pid = kd->kvm_pent.flpid;
       mpid = mapPID(kd->pid);
       kd->kvm_pent.parentpid =
           kp->procTable[kp->procTable[mpid].parentpid].flpid;
       i = kd->procIndex+1;
       while ((i < NPROC) && (kp->procTable[i].processState == procUNUSED))
              i++;
       kd->procIndex = i;
       return &kd->kvm_pent;
   }
   else return NULL;
}

int KERNkvmsetproc(int *ERRNO,kvmt *kd)
{
int i;

    for (i = 0; i < NPROC; i++)
        if (kp->procTable[i].processState != procUNUSED) {
            kd->procIndex = i;
            return OK;
        }
    return SYSERR;
}

int KERNtcnewpgrp(int *ERRNO, int fdtty)
{
unsigned i,devNum,ttyPgrp;
fdentryPtr tty;
extern fdentryPtr getFDptr(int);
   
   tty = getFDptr(fdtty);
   if ((tty == NULL) || (tty->refNum == 0)) { *ERRNO = EBADF; return -1; }
   if (tty->refType != rtTTY) { *ERRNO = ENOTTY; return -1; }
   devNum = tty->refNum-1;
   disableps();

   for (i = 0; i < NPGRP; i++)
       if (!pgrpInfo[i].pgrpref) break;

   if (i == NPGRP) { enableps(); return -1; }

   /* we're leaving a PTY, decrement the refcount */
   if (ttyPgrp = ttys[devNum].pgrp)
       pgrpInfo[ttyPgrp-2].pgrpref--;
   if (kp->gsosDebug & 16)
       fprintf(stderr, "tcnewpgrp- TTY:%d  pgrp:%d\n",devNum,i+2);
   ttys[devNum].pgrp = i+2;
   pgrpInfo[i].pgrpref++;
   enableps();
   return 0;
}

int KERNsettpgrp(int *ERRNO, int fdtty)
{
int p,pid = Kgetpid();
int pp,devNum;
fdentryPtr tty;
extern fdentryPtr getFDptr(int);

    tty = getFDptr(fdtty);
    if ((tty == NULL) || (tty->refNum == 0)) { *ERRNO = EBADF; return -1; }
    if (tty->refType != rtTTY) { *ERRNO = ENOTTY; return -1; }
    devNum = tty->refNum-1;
    p = ttys[devNum].pgrp;
    if (kp->gsosDebug & 16)
    	fprintf(stderr, "settpgrp pid: %d, oldpgrp: %d, ",
        	PROC->flpid, PROC->pgrp);
    /* $$$   kp->procTable[pid].flpid,
       kp->procTable[pid].pgrp); */

    disableps();
/* $$$   if (kp->procTable[pid].pgrp != p) */
    if (PROC->pgrp != p)
    {
	if (p != 0) pgrpInfo[p-2].pgrpref++;
/* $$$       if ((pp = kp->procTable[pid].pgrp) != 0) */
	if ((pp = PROC->pgrp) != 0)
          pgrpInfo[pp-2].pgrpref--;
    }
    if (kp->gsosDebug & 16) fprintf(stderr,"newpgrp: %d\n",p);
    /* $$$ kp->procTable[pid].pgrp = p; */
    PROC->pgrp = p;
    enableps();
}

int KERNtctpgrp(int *ERRNO, int pid, int fdtty)
{
int p,mpid,devNum;
fdentryPtr tty;
extern fdentryPtr getFDptr(int);

   /* should check pid to be sure it's == getpid or a child of it */
   mpid = mapPID(pid);
   if (mpid == -1) { *ERRNO = ESRCH; return -1; }
/*
   if (!((mpid == Kgetpid()) || (kp->procTable[mpid].parentpid == Kgetpid())))
       { *ERRNO = ERANGE; return -1; }
*/
   tty = getFDptr(fdtty);
   if ((tty == NULL) || (tty->refNum == 0)) { *ERRNO = EBADF; return -1; }
   devNum = tty->refNum-1;
   if (tty->refType != rtTTY) { *ERRNO = ENOTTY; return -1; }
   disableps();
   p = kp->procTable[mpid].pgrp;
   if (p != ttys[devNum].pgrp) {
       if (p != 0) pgrpInfo[p-2].pgrpref++;
       if (ttys[devNum].pgrp != 0)
           pgrpInfo[ttys[devNum].pgrp-2].pgrpref--;
   }
   if (kp->gsosDebug & 16)
       fprintf(stderr,"tctpgrp TTY:%d pid:%d pgrp:%d\n",devNum,pid,p);
   ttys[devNum].pgrp = p;
   enableps();
}

int KERNsetdebug(int code)
{
int old;

    if (kp->gsosDebug & 16) fprintf(stderr,"setdebug %d\n",code);
    if ((code < 0) || (code > 63)) return SYSERR;
    old = kp->gsosDebug;
    kp->gsosDebug = code;
    return old;
}

void *KERNsetsystemvector(void * execvec)
{
void *x;
    x = PROC->executeHook;
    PROC->executeHook = execvec;

 /* $$$   x = kp->procTable[Kgetpid()].executeHook;
    kp->procTable[Kgetpid()].executeHook = execvec;  */
    return x;
}

#ifdef NOTDEFINED
struct pipest {
    handle bufferH;         /* the pipe data handle */
    word   in;              /* write pointer        */
    word   out;             /* read pointer         */
    word   qflags;
    word   RrnCount;        /* Why? Because special stuff has to */
    word   WrnCount;        /* happen in strange conditions      */

    int    accessSem;
    int    readSem;
    int    writeSem;
    int    writeStatus;
}
#endif

/* filedes[0] is read only, filedes[1] is write only */
int KERNpipe(int *ERRNO, int filedes[2])
{
fdentryPtr pread,pwrite;
int fdread,fdwrite;
fdtablePtr ft;
int pipen;
extern int newPipe(void);

    if (kp->gsosDebug & 16) printf("pipe(%06lX)\n",filedes);
  /* $$$  ft = kp->procTable[Kgetpid()].openFiles; */
    pipen = newPipe();
    pread = allocFD(&fdread);
    if (pread == NULL) { *ERRNO = EMFILE; disposePipe(pipen); return -1; }
    pwrite = allocFD(&fdwrite);
    ft = PROC->openFiles; /* deref pointer after calls to allocFD */
    if (pwrite == NULL) {
        pread->refNum = 0; /* deallocate the read file descriptor */
        *ERRNO = EMFILE;
        disposePipe(pipen);
        return -1;
    }
    disableps();
    filedes[0] = fdread; filedes[1] = fdwrite;
	/* a->i1 = a->i2 = b BROKEN in C 2.0.3 */
#if 0
    pread->refNum = pwrite->refNum = pipen;
    pread->refType = pwrite->refType = rtPIPE;
    pread->refLevel = pwrite->refLevel = ft->fdLevel | ft->fdLevelMode;
    pwrite->NLenableMask = pread->NLenableMask = 0;
#else
    pread->refNum = pipen;	pwrite->refNum = pipen;
    pread->refType = rtPIPE;	pwrite->refType = rtPIPE;
    pwrite->refLevel = ft->fdLevel | ft->fdLevelMode;
    pread->refLevel = pwrite->refLevel;
    pwrite->NLenableMask = 0;	pread->NLenableMask = 0;
#endif
    pread->refFlags = rfPIPEREAD;
    pwrite->refFlags = rfPIPEWRITE;
    
    AddRefnum(rtPIPE,pipen);
    IncRefnum(rtPIPE,pipen);
    ft->fdCount += 2; /* we opened two new files */
    enableps();
    return 0;
}

int KERNdup(int *ERRNO, int filedes)
{
/* $$$ struct pentry *p; */
fdtablePtr ft;
fdentryPtr newFD,oldFD;
int nfd;
int i,j,fd;
extern void IncRefnum(int,int);

    disableps();
  /* $$$  p = &(kp->procTable[Kgetpid()]); */
    fd = filedes-1;
    ft = PROC->openFiles;
    if ((filedes < 1) || (filedes > 32) ||
        (ft->fds[fd].refNum == 0)) {
        *ERRNO = EBADF;
		enableps();
        return -1;
    }

    newFD = allocFD(&nfd);
    ft = PROC->openFiles;
    if (newFD == NULL) { *ERRNO = EMFILE; enableps(); return -1; }
    oldFD = &(ft->fds[fd]);
    memcpy(newFD,oldFD,sizeof(fdentry));
    IncRefnum(newFD->refType,newFD->refNum);
    if (newFD->refType == rtPIPE) 
        incPipe(newFD->refFlags,newFD->refNum);
    ft->fdCount++; /* we created a new one... */
    enableps();
    return nfd;
}

int KERNdup2(int *ERRNO, int filedes2, int filedes)
{
/* $$$ struct pentry *p; */
fdtablePtr ft;
fdentryPtr newFD,oldFD;
int i,j,fd,fd2;
extern void IncRefnum(int,int);
int cl[2];

    disableps();
 /* $$$   p = &(kp->procTable[Kgetpid()]); */
    ft = PROC->openFiles;
    fd = filedes-1;  fd2 = filedes2-1;
    newFD = &(ft->fds[fd2]); oldFD = &(ft->fds[fd]);
    if ((filedes < 1) || (filedes2 < 1) || (filedes > 32) || (filedes2 > 32) ||
        (oldFD->refNum == 0)) {
        *ERRNO = EBADF;
        enableps();
        return -1;
    }
    /* if they're duping a file to itself, pretend we did it */
    if (fd == fd2) {
        enableps(); return 0;
    }
    if (newFD->refNum) {
        cl[0] = 1; cl[1] = filedes2;
        CloseGS(cl);
    }
    memcpy(newFD,oldFD,sizeof(fdentry));
    IncRefnum(newFD->refType,newFD->refNum);
    if (newFD->refType == rtPIPE)
        incPipe(newFD->refFlags,newFD->refNum);
    ft->fdCount++; /* we created a new one... */
    enableps();
    return 0;
}

#pragma databank 0
#pragma toolparms 0

