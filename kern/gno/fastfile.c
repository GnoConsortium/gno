/*	$Id: fastfile.c,v 1.1 1998/02/02 08:18:27 taubert Exp $ */

/*

   fastfile.c

   C routines to duplicate the Orca Shell fastfile system

   I wouldn't do this except the Orca compilers require FastFile.

*/
#pragma optimize 72

#include "proc.h"
#include "sys.h"
#include "/lang/orca/libraries/orcacdefs/stdio.h"
#include "/lang/orca/libraries/orcacdefs/string.h"
#include "/lang/orca/libraries/orcacdefs/stdlib.h"
#include "/lang/orca/libraries/orcacdefs/stddef.h"
#include <ctype.h>
#include <gsos.h>
#include <shell.h>
#include <memory.h>
#include <misctool.h>
#include <orca.h>

typedef struct {
    int action;
    int index;
    int flags;  
    handle fileHandle;
    GSString255Ptr pathName;
    int access;
    int fileType;
    long auxType;
    int storageType;
    TimeRec createDate;
    TimeRec modDate;
    void *option;
    long fileLength;
    long blocksUsed;
} my_FastFileGSPB;

#pragma lint -1

void printGS(GSString255Ptr path);

/*#define DEBUG*/
#ifdef DEBUG
#define PRINT(arg) fprintf(stderr,arg)
#else
#define PRINT(arg)
#endif

segment "KERN3     ";
extern void gnoSetHandleID(word, handle);

struct ffentry {
    int action;
    int index;
    int flags;
    handle fileHandle;
    GSString255Ptr pathnameGS;
    char *pathnameP;
    FileInfoRecGS info;
    int hidden;
    struct ffentry *next;
};
typedef struct ffentry ffentry, *ffentryPtr;

/*extern int OLDGSOSST(word callnum, void *pBlock); */

ffentryPtr ffList = NULL;
unsigned InFastFile = 0;

int pstrlen(char *p)
{ return p[0]; }

extern void copygs2res(void *,void*);

void p2gs(char *src, GSString255Ptr gs)
{
   gs->length = pstrlen(src);
   memcpy(gs->text,src+1,gs->length);
}

ffentryPtr NewFF(GSString255Ptr pathnameGS)
{
GSString255Ptr pathGS;
char *path;
ffentryPtr newEntry;
int tmp;

    path = malloc((size_t) (pathnameGS->length+1));
    path[0] = pathnameGS->length;
    memcpy(path+1,pathnameGS->text,(size_t) path[0]);
    /* perhaps we should change the seps AFTER we copy the contents of
       the string */
    for (tmp = 1; tmp <= path[0]; tmp++)
    	if (path[tmp] == ':') path[tmp] = '/';
    pathGS = malloc((size_t) (pathnameGS->length+2));
    pathGS->length = pathnameGS->length;
    memcpy(pathGS->text,pathnameGS->text,pathGS->length);
    newEntry = malloc(sizeof(ffentry));

/* link into the list */
    newEntry->next = ffList;
    ffList = newEntry;

    newEntry->pathnameP = path;
    newEntry->pathnameGS = pathGS;
    newEntry->fileHandle = NULL;
    newEntry->hidden = 0;
    return newEntry;
}

#pragma databank 1
void DeleteFF(ffentryPtr p)
{
ffentryPtr prev;

   prev = ffList;
   if (prev != p)
       while (prev != NULL) {
           if (prev->next == p) break;
           prev = prev->next;
       }
   if (prev == NULL) { fprintf(stderr,"entry not found in DeleteFF\n");
                       return; }
   if (p->pathnameGS != NULL) nfree(p->pathnameGS);
   if (p->pathnameP != NULL) nfree(p->pathnameP);
   if (p->fileHandle != NULL) {
   	HUnlock(p->fileHandle);
    	DisposeHandle(p->fileHandle);
   }
   if (prev == p) { ffList = prev->next; }
   else { prev->next = p->next; }
   nfree(p);
}

ffentryPtr FindIndFF(int index)
{
ffentryPtr p;
int step = 0;

   p = ffList;
   while (p != NULL) {
       if (step == index) break;
       step++;
       p = p->next;
   }
   return p;
}

ffentryPtr FindFF(GSString255Ptr path)
{
ffentryPtr p;
extern int GScaseEqual(void*,void*);

   p = ffList;
   while (p != NULL) {
       if (GScaseEqual(path,p->pathnameGS) && (!p->hidden) ) break;
       p = p->next;
   }
   return p;
}
#pragma databank 0

FileInfoRecGS fi;
CreateRecGS cr;
NameRecGS ds;
OpenRecGS op;
IORecGS re;
ExpandPathRecGS pcep;
int cl[2];

int upToDate(ffentryPtr p)
{
FileInfoRecGS f;
word dateBuf[8];

   if (p->flags & 0x8000) {
       f.pCount = 7;
       f.pathname = p->pathnameGS;
       GetFileInfoGS(&f);
       if (toolerror()) printf("GetFileInfo err %04X\n",toolerror());
       if (memcmp(&f.modDateTime,&p->info.modDateTime,sizeof(TimeRec)))
           return 0;
/*       memcpy(dateBuf,&f.modDateTime,sizeof(TimeRec));
       ConvSeconds(9,0l,(Pointer)dateBuf);
       if ((dateBuf[0] != p->info.mod_date) ||
           (dateBuf[1] != p->info.mod_time)) return 0;  */
/*       memcpy(dateBuf,&f.createDateTime,sizeof(TimeRec));
       ConvSeconds(9,0l,(Pointer)dateBuf);
       if ((dateBuf[0] != p->info.create_date) ||
           (dateBuf[1] != p->info.create_time)) return 0; */
   }
   return 1;
}

void ffErr(int err)
{
/*   fprintf(stderr,"FastFile error: %04X\n",err); */
}

handle loadFile(GSString255Ptr pathGS, ffentryPtr p)
{
handle fileHandle;
word dateBuf[8];
char asciiBuf[30];
longword secs;
int err;

    p->hidden = 1;
    op.pCount = 13;
    op.pathname = pathGS;
    op.requestAccess = readEnable;
    op.resourceNumber = 0;
    op.optionList = NULL;
    /* err = OLDGSOSST(0x2010,&op); */
    /* if (err) { ffErr(err); return NULL; } */
    OpenGS(&op); /* to prevent conflict with Open-remove code */
    if (_toolErr) { ffErr(_toolErr); p->hidden = 0; return NULL; }
    fileHandle = NewHandle(op.eof,userid() | 0x0100, 0x8000, NULL);
    if (fileHandle == NULL) { p->hidden = 0; ffErr(toolerror()); return NULL; }
    re.pCount = 4;
    re.refNum = op.refNum;
    re.dataBuffer = *fileHandle;
    re.requestCount = op.eof;
    /*err = OLDGSOSST(0x2012,&re);*/
    ReadGS(&re);
    if (err) { ffErr(err); }
    cl[0] = 1;
    cl[1] = op.refNum;
    CloseGS(cl);
    /*err = OLDGSOSST(0x2014,cl);*/
    p->fileHandle = fileHandle;
    p->info.eof = op.eof;
    p->info.blocksUsed = op.blocksUsed;
    p->info.resourceEOF = op.resourceEOF;
    p->info.resourceBlocks = op.resourceBlocks;
    p->info.access = op.access;
    p->info.fileType = op.fileType;
    p->info.auxType = op.auxType;
    p->info.storageType = op.storageType;
    memcpy(&p->info.createDateTime,&op.createDateTime,sizeof(TimeRec));
    memcpy(&p->info.modDateTime,&op.modDateTime,sizeof(TimeRec));

/*    memcpy(dateBuf,&op.createDateTime,sizeof(TimeRec));
    ConvSeconds(9,0l,(Pointer)dateBuf);
    p->info.create_date = dateBuf[0];
    p->info.create_time = dateBuf[1];
    memcpy(dateBuf,&op.modDateTime,sizeof(TimeRec));
    ConvSeconds(9,0l,(Pointer)dateBuf);
    p->info.mod_date = dateBuf[0];
    p->info.mod_time = dateBuf[1];

    p->info.blocks_used = op.blocksUsed; */
    p->hidden = 0;
    return fileHandle;
}

/*
   the entry point for all fastFile routines.  The assembly code just
   calls fastEntry with the address of the paramBlock (real one, not the
   copy)

   osFlag = 0 for P16, 1 for GS/OS strings
   ff must point to the first parameter, not the pCount in the new scheme
   osFlag will always be 0 for now.  I won't guess at what the Orca 2.0
    call format's going to be.
*/

#pragma databank 1
int fastEntry(FastFilePB *ff, int osFlag, int pCount)
{
my_FastFileGSPB *fg = (my_FastFileGSPB *) ff;
ffentryPtr p;
handle fileHandle,tmpHandle;
GSString255Ptr pathCopy;
ResultBuf255Ptr epRes;
int tmp,err;
char *tmps;
int dateBuf[4];
extern void printGS(GSString255Ptr path);

   err = 0;
/* all calls but Indexed Load take path as input, so condition it */
   InFastFile = 1;
   if (ff->action != 1) {
       if (!osFlag) {
           pathCopy = malloc(pstrlen(ff->pathname)+2);
           p2gs(ff->pathname,pathCopy);
           pcep.inputPath = pathCopy;
       } else pcep.inputPath = fg->pathName;

       epRes = malloc(sizeof(ResultBuf255));
       epRes->bufSize = 255;
       pcep.outputPath = epRes;
       pcep.pCount = 3;
       pcep.flags = (!osFlag) ? 0x8000 : 0;
       ExpandPathGS(&pcep);
       if (!osFlag) nfree(pathCopy);
       pathCopy = &epRes->bufString;
 /*    for (tmp = 0; tmp < pathCopy->length; tmp++)
       if (pathCopy->text[tmp] == ':') pathCopy->text[tmp] = '/';  */
    }
    switch (ff->action) {
       case 0: /* Load */
       case 2: /* Load from Memory */
#ifdef DEBUG
           if (ff->action == 0) printf("Load "); else printf("Load Memory ");
           printGS(pathCopy); printf("\n");
#endif
           p = FindFF(pathCopy); /* find matching entry */
           if (p == NULL) {
               if (ff->action == 2) { err = 0x46; break; } /* file not found */
               else p = NewFF(pathCopy); /* make a new entry */
           }

           if ((p->fileHandle != NULL) && /* handle allocated? */
              (*p->fileHandle != NULL) && /* handle purged?    */
              (upToDate(p)))               /* memory copy up to date? */
              {
#ifdef DEBUG
printGS(p->pathnameGS);
printf(" up to date\n");
#endif
                   HLock(p->fileHandle);
                   /*if (!osFlag) ff->pathname = p->pathnameP;*/
                   goto setpath; /* set the pathname */
                   break;
              }
           /* handle was purged or image was not up to date.  We dispose
              here and loadFile will allocate a new one */
           if (p->fileHandle != NULL) DisposeHandle(p->fileHandle);

           fileHandle = loadFile(pathCopy,p);
           if (fileHandle == NULL) {
               err = toolerror();
               DeleteFF(p);
           }
           else p->flags = ff->flags;
setpath:   if (!osFlag)
               ff->pathname = p->pathnameP;

        /* printf("Load(Mem): flags:%04X\n",p->flags);*/
           break;

/* Mike is totally off his rocker.  FastFile $01 returns a pointer to a
   resultBuffer.  Hopefully the new version uses a GS/OS resultBuf. */

       case 1: /* Indexed Load */
           PRINT("<indexed load ");
#ifdef DEBUG
           printf("(%d)> ",ff->index);
#endif
retryil:
           p = FindIndFF(ff->index); /* find matching entry */
           if (p == NULL) { err = 0x46; break; }
           if ((*p->fileHandle != NULL)) { /*&& /* handle purged?    */
   /*           (upToDate(p))) {    */          /* memory copy up to date? */
                 if (!osFlag)
                     ff->pathname = p->pathnameP;
                 else { copygs2res(fg->pathName,p->pathnameGS); }
                 HLock(p->fileHandle);
                 ff->flags = p->flags;
                 break;
           /* Mike copies it to a result buffer, but that's not safe in
              a concurrent environment.  This will do, as everyone assumes
              you can't write to the pathname field */
           }
           /* handle was purged or image was not up to date.  We dispose
              here and loadFile will allocate a new one */
           DisposeHandle(p->fileHandle);
           /* if the file was not on disk, do not attempt to load it! 
              remove it from the FF system and retry the Indexed_Load call */
       /*    if (!(p->flags & 0x8000)) {    */
               DeleteFF(p);
               goto retryil;
    /*       }    */
           fileHandle = loadFile(p->pathnameGS,p);
           if (fileHandle == NULL) { InFastFile = 0; return toolerror(); }
           ff->flags = p->flags;
           if (!osFlag)
               ff->pathname = p->pathnameP;
           else { copygs2res(fg->pathName,p->pathnameGS); }
           break;

       case 3: /* Save */
           PRINT("Save:");
#ifdef DEBUG
           printGS(pathCopy); printf("\n");
#endif
           p = FindFF(pathCopy);
           if (p != NULL) {
               PRINT("save:file existed\n");
               if (ff->file_handle == p->fileHandle)
                   p->fileHandle = NULL; /* don't kill the block */
               DeleteFF(p);
           }
           p = NewFF(pathCopy);   /* create a new entry */
           p->flags = ff->flags;
           p->fileHandle = ff->file_handle;
           p->info.eof = (osFlag ? fg->fileLength : ff->file_length);
           cr.access = p->info.access = (osFlag ? fg->access : ff->access);
           cr.fileType = p->info.fileType =
               (osFlag ? fg->fileType : ff->file_type);
           cr.auxType = p->info.auxType =
               (osFlag ? fg->auxType : ff->aux_type);

           if (!osFlag) {
               dateBuf[0] = ff->create_date;
               dateBuf[1] = ff->create_time;
               ConvSeconds(8,0l,(Pointer)dateBuf);
               memcpy(&p->info.createDateTime,dateBuf,sizeof(TimeRec));
               dateBuf[0] = ff->mod_date;
               dateBuf[1] = ff->mod_time;
               ConvSeconds(8,0l,(Pointer)dateBuf);
               memcpy(&p->info.modDateTime,dateBuf,sizeof(TimeRec));
           } else {
               memcpy(&p->info.modDateTime,&fg->modDate,sizeof(TimeRec));
               memcpy(&p->info.createDateTime,&fg->createDate,sizeof(TimeRec));
           }
           cr.pCount = 4;
           cr.pathname = p->pathnameGS;
doCreate:
           CreateGS(&cr);
           if (toolerror()) { err = toolerror();
               if (err != 0x47) { DeleteFF(p); break; }
               err = 0;
               ds.pCount = 1;
               ds.pathname = pathCopy;
                p->hidden = 1;
                 DestroyGS(&ds);
                p->hidden = 0;
               if (!_toolErr) goto doCreate;
               /*err = OLDGSOSST(0x2002,&ds);*/
               /*if (!err) goto doCreate;*/
               err = _toolErr; DeleteFF(p); break;
           }
           op.pCount = 2;
           op.pathname = p->pathnameGS;
           p->hidden = 1;
           OpenGS(&op);
           if (_toolErr) { err = _toolErr; DeleteFF(p); p->hidden = 0; break; }
           /*err = OLDGSOSST(0x2010,&op);
           if (err) { DeleteFF(p); p->hidden = 0; break; }  */
           re.pCount = 4;
           re.refNum = op.refNum;
           re.dataBuffer = *p->fileHandle;
           re.requestCount = p->info.eof;
           /*err = OLDGSOSST(0x2013,&re);*/
           WriteGS(&re);
           cl[0] = 1; cl[1] = op.refNum;
           CloseGS(cl);
           /*OLDGSOSST(0x2014,cl);*/
           p->hidden = 0;
           fi.pCount = 10;
           fi.optionList = NULL;
           fi.pathname = p->pathnameGS;
           GetFileInfoGS(&fi);
           p->info.blocksUsed = fi.blocksUsed;
       /*  tmpHandle = NewHandle(GetHandleSize(p->fileHandle),
               userid() | 0x0100, 0x8000, NULL);
           HandToHand(p->fileHandle,tmpHandle,
               GetHandleSize(p->fileHandle)); */
           /*p->fileHandle = tmpHandle;*/
           gnoSetHandleID(userid(),p->fileHandle);
           err = 0; break;

       case 4: /* Add */
           PRINT("Add ");
#ifdef DEBUG
           printGS(pathCopy); printf("\n");
#endif
           p = FindFF(pathCopy); /* is there already one? */
           if (p != NULL)
               DisposeHandle(p->fileHandle);
           else p = NewFF(pathCopy);
           if (!osFlag) {
              p->info.access = ff->access;
              p->info.fileType = ff->file_type;
              p->info.auxType = ff->aux_type;
              p->info.storageType = ff->storage;
              dateBuf[0] = ff->create_date;
              dateBuf[1] = ff->create_time;
              ConvSeconds(8,0l,(Pointer)dateBuf);
              memcpy(&p->info.createDateTime,dateBuf,sizeof(TimeRec));
              dateBuf[0] = ff->mod_date;
              dateBuf[1] = ff->mod_time;
              ConvSeconds(8,0l,(Pointer)dateBuf);
              memcpy(&p->info.modDateTime,dateBuf,sizeof(TimeRec));
              p->info.blocksUsed = ff->blocks_used;
              p->info.eof = ff->file_length;
           } else {
               memcpy(&p->info.access,&fg->access,36l);
           }
           p->flags = ff->flags;
           p->fileHandle = NewHandle(GetHandleSize(fg->fileHandle),
               userid() | 0x0100, 0x8000, NULL);
           HandToHand(fg->fileHandle,p->fileHandle,
               GetHandleSize(fg->fileHandle));

      /*   printf("Add: flags: %04X,handle:$%06lX\n",p->flags,
             p->fileHandle);   */
           if (p->flags & 0x4000)
               SetPurge(2,p->fileHandle);
           else SetPurge(0,p->fileHandle);
           break;

       case 5:
       case 6:
#ifdef DEBUG
           if (ff->action == 5) printf("Delete ");else printf("Remove ");
           printGS(pathCopy); printf("\n");
#endif
           p = FindFF(pathCopy); /* find the file */
           if (p == NULL) { err = 0x46; break; }
           if (ff->action == 6) p->fileHandle = NULL;
           DeleteFF(p);
           break;

       case 7: /* Purge */
           PRINT("<Purge ");
#ifdef DEBUG
           printGS(pathCopy); printf("> ");
#endif
           p = FindFF(pathCopy); /* find matching entry */
           if (p == NULL) { err = 0x46; break; } /* file not found */
           if (p->flags & 0x4000) {
               SetPurge(2,p->fileHandle);
               HUnlock(p->fileHandle);
           }
           break;
   }
   if (ff->action != 1) nfree(epRes);
   if (!err)
     switch (ff->action) {
       case 0:  /* argh! why'd Mike stick Pathname in the middle of the */
       case 1:  /* return values instead of the beginning? */
       case 2:
         if (!osFlag) {
           ff->file_handle = p->fileHandle;
           ff->file_length = p->info.eof;
           ff->access = p->info.access;
           ff->file_type = p->info.fileType;
           ff->aux_type = p->info.auxType;
           ff->storage = p->info.storageType;

           memcpy(dateBuf,&p->info.createDateTime,sizeof(TimeRec));
           ConvSeconds(9,0l,(Pointer)dateBuf);
           ff->create_date = dateBuf[0];
           ff->create_time = dateBuf[1];
           memcpy(dateBuf,&p->info.modDateTime,sizeof(TimeRec));
           ConvSeconds(9,0l,(Pointer)dateBuf);
           ff->mod_date = dateBuf[0];
           ff->mod_time = dateBuf[1];

/*           ff->create_date = p->info.createDate;
           ff->create_time = p->info.createTime;
           ff->mod_date = p->info.modDate;
           ff->mod_time = p->info.modTime;  */
           ff->blocks_used = p->info.blocksUsed;
           if (ff->action == 1) ff->flags = p->flags;
         }
         else switch (pCount) {
           case 14: fg->blocksUsed = p->info.blocksUsed;
           case 13: fg->fileLength = p->info.eof;
           case 12: fg->option = p->info.optionList;
           case 11: memcpy(&fg->modDate,&p->info.modDateTime,sizeof(TimeRec));
           case 10: memcpy(&fg->createDate,&p->info.createDateTime,
                        sizeof(TimeRec));
           case  9: fg->storageType = p->info.storageType;
           case  8: fg->auxType = p->info.auxType;
           case  7: fg->fileType = p->info.fileType;
           case  6: fg->access = p->info.access;
           default: fg->fileHandle = p->fileHandle;
           if (ff->action == 1) fg->flags = p->flags;
         }
     }
/*   if (err) printf("[err:%04X] ",err); */
   InFastFile = 0;
   return err;
}

extern kernelStructPtr kp;

typedef struct {
    char *sFile;
    char *dFile;
    char *parms;
    char *iString;
    byte merr;
    byte merrf;
    byte lops;
    byte kflag;
    unsigned long mFlags;
    unsigned long pFlags;
    unsigned long org;
} oLInfoPB;

typedef struct {
    GSString255Ptr sFile;
    GSString255Ptr dFile;
    GSString255Ptr parms;
    GSString255Ptr iString;
    byte merr;
    byte merrf;
    byte lops;
    byte kflag;
    unsigned long mFlags;
    unsigned long pFlags;
    unsigned long org;
} oLInfoRec;

void convcol2sl(char *p)
{
int i;

    for (i = 1; i <= p[0]; i++)
        if (p[i] == ':') p[i] = '/';
}

int cSetLInfo(oLInfoPB *pBlock, int cmdNum)
{
oLInfoRec *tmp;
extern void copyp2gs(void *, void *);
extern void copygsstr(void *, void *);

  /* $$$ tmp = kp->procTable[Kgetpid()].LInfo; */
    tmp = PROC->LInfo;
    if (tmp == NULL) { tmp = malloc(sizeof(oLInfoRec));
       /* $$$ kp->procTable[Kgetpid()].LInfo = tmp; */
        PROC->LInfo = tmp;
	/* a->i1 = a->i2 = b BROKEN in C 2.0.3 */
#if 0
	tmp->sFile = tmp->dFile = tmp->parms = tmp->iString = NULL;
#else
	tmp->sFile = NULL;
	tmp->dFile = NULL;
	tmp->parms = NULL;
	tmp->iString = NULL;
#endif
    }
    if (tmp->sFile != NULL) {
        nfree(tmp->sFile);
        nfree(tmp->dFile);
        nfree(tmp->iString);
        nfree(tmp->parms);
    }
    if (cmdNum == 0x0102) {
        tmp->sFile = malloc(pBlock->sFile[0] + 2);
        copyp2gs(tmp->sFile,pBlock->sFile);
        tmp->dFile = malloc(pBlock->dFile[0] + 2);
        copyp2gs(tmp->dFile,pBlock->dFile);
        tmp->parms = malloc(pBlock->parms[0] + 2);
        copyp2gs(tmp->parms,pBlock->parms);
        tmp->iString = malloc(pBlock->iString[0] + 2);
        copyp2gs(tmp->iString,pBlock->iString);
    } else {
        tmp->sFile = malloc( ((oLInfoRec *) pBlock)->sFile->length + 2);
        copygsstr(tmp->sFile,pBlock->sFile);
        tmp->dFile = malloc( ((oLInfoRec *) pBlock)->dFile->length + 2);
        copygsstr(tmp->dFile,pBlock->dFile);
        tmp->parms = malloc( ((oLInfoRec *) pBlock)->parms->length + 2);
        copygsstr(tmp->parms,pBlock->parms);
        tmp->iString = malloc( ((oLInfoRec *) pBlock)->iString->length + 2);
        copygsstr(tmp->iString,pBlock->iString);
    }
    tmp->merr = pBlock->merr;
    tmp->merrf = pBlock->merrf;
    tmp->lops = pBlock->lops;
    tmp->kflag = pBlock->kflag;
    tmp->mFlags = pBlock->mFlags;
    tmp->pFlags = pBlock->pFlags;
    tmp->org = pBlock->org;
    return 0;
}

int cGetLInfo(oLInfoPB *pBlock, int cmdNum)
{
oLInfoRec *tmp;
extern void copygs2p(void *, void *);
extern void copygs2res(void *, void *);

  /* $$$  tmp = kp->procTable[Kgetpid()].LInfo; */
    tmp = PROC->LInfo;
    if (tmp == NULL) return 0;
    if (cmdNum == 0x0101) {
        copygs2p(pBlock->sFile,tmp->sFile);
        convcol2sl((char *)pBlock->sFile);
        copygs2p(pBlock->dFile,tmp->dFile);
        convcol2sl((char *)pBlock->dFile);
        copygs2p(pBlock->iString,tmp->iString);
        copygs2p(pBlock->parms,tmp->parms);
    }
    else {
        copygs2res(pBlock->sFile,tmp->sFile);
        copygs2res(pBlock->dFile,tmp->dFile);
        copygs2res(pBlock->iString,tmp->iString);
        copygs2res(pBlock->parms,tmp->parms);
    }
    pBlock->merr = tmp->merr;
    pBlock->merrf = tmp->merrf;
    pBlock->lops = tmp->lops;
    pBlock->kflag = tmp->kflag;
    pBlock->mFlags = tmp->mFlags;
    pBlock->pFlags = tmp->pFlags;
    pBlock->org = tmp->org;
    return 0;
}

#pragma databank 0

