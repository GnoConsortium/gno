/*	$Id: main.c,v 1.1 1998/02/02 08:18:33 taubert Exp $ */

segment "KERN2     ";

#pragma stacksize 1024
#pragma optimize 79

#include "proc.h"
#include "gno.h"
#include "sys.h"
#include "sem.h"
#include "tty.h"
#include "/lang/orca/libraries/orcacdefs/stdio.h"
#include "/lang/orca/libraries/orcacdefs/stdlib.h"
#include "/lang/orca/libraries/orcacdefs/string.h"
#include <memory.h>
#include <gsos.h>
#include <event.h>
#include <locator.h>
#include <loader.h>
#include <texttool.h>
#include <orca.h>
#include <sys/ioctl.h>
#include <sys/ports.h>
#include <shell.h>
#include <sys/errno.h>
#include <signal.h>

#ifndef udispatch
#define udispatch 0xE10008
#endif
extern pascal int   kernStatus() inline(0x0603, udispatch);

struct pentry *procPtr;

#ifdef UNUSED_CODE
static void prResultString(ResultBuf255Ptr r)
{
int i;

    putchar('(');
    for (i = 0; i < r->bufString.length; i++) putchar(r->bufString.text[i]);
    putchar(')');
    printf("[%d]",r->bufString.length);
    putchar('\n');
}
#endif

kernelStructPtr kp;
int sysR;
char initX[80] = "9/gsh", initCmd[80] = "gsh";

static void doShell(void)
{
int cl[2];
GSString32 ttyPath = { 6, ".ttyco" };
OpenRecGS o;
FILE *initf;

#ifdef DEBUG_FIRST_FORK
    WriteCString("\n\rInside doShell\n\r");
    ReadChar(0);
#endif
    /* we don't want processes to inherit the sys.resources fd */
    cl[0] = 1; cl[1] = 0; CloseGS(cl);

    o.pCount = 3;
    o.pathname = (GSString255Ptr) &ttyPath; /* our console driver mabob */
    o.requestAccess = readEnable;
    OpenGS(&o);	/* open stdin */
    o.requestAccess = writeEnable;          
    OpenGS(&o);  /* open stdout */
    o.requestAccess = writeEnable;
    OpenGS(&o);  /* open stderr */

    initf = fopen("9/initrc","r");
    if (initf) {
	fgets(initX,80,initf);
	initX[strlen(initX)-1] = 0;	/* take off trailing \r */
	fgets(initCmd,80,initf);
	initCmd[strlen(initCmd)-1] = 0;	/* take off trailing \r */
	fclose(initf);
    }

    cl[0] = 0;
    PUSH_VARIABLES(cl);
#ifdef DEBUG_FIRST_FORK
    WriteCString("Calling Kexecve()\n\r");
    ReadChar(0);
#endif
    Kexecve(&errno,initCmd,initX);
    WriteCString("\n\rCould not locate: ");
    WriteCString(initX);
    WriteCString("\n\r");
    if (initf)
	WriteCString("error in INITRC configuration file\n\r");
    else
	WriteCString("could not find INITRC configuration file\n\r");
    WriteCString("Press a key to exit GNO\n\r");
    ReadChar(0);
}

GSString255Ptr __C2GSMALLOC(char *s)
{
GSString255Ptr g;
size_t l;

    l = strlen(s);
    g = malloc(l+2);
    g->length = l;
    memcpy(g->text,s,l);
    return g;
}

static int numDrivers;
static int driverUserID[16];
GSString255Ptr DeviceNames[40];

extern void InstallDriver(int,int,void *);

static void setuppty(void)
{
char pty[] = ".ptyq0";
char tty[] = ".ttyq0";
const char conv[] = "0123456789abcdef";
unsigned int ptyno, slotno;
extern PTYMastHeader;
extern PTYSlaveHeader;

    slotno = 6;
    for (ptyno = 0; ptyno < 16; ptyno++) {
        tty[5] = pty[5] = conv[ptyno];
        DeviceNames[slotno] = __C2GSMALLOC(pty);
    	DeviceNames[slotno+1] = __C2GSMALLOC(tty);
    	InstallDriver(kp->userID, slotno, &PTYMastHeader);
    	InstallDriver(kp->userID, slotno+1, &PTYSlaveHeader);
	slotno+=2;
    }
}

static void setuptty(void)
{
static char line[80];
char *line1;
FILE *ttys;
word ILuserID;
InitialLoadOutputRec il_rec;
int e;
static GSString255 filename;
int devNum;
static char devname[20];
extern ConsoleHeader;

    numDrivers = 0;

    strcpy(line,"9:dev:"); /* all loaded devices are referenced from 9:dev */
    line1 = line+6;
    setuppty(); /* install 16 pty devices */

    /* .ttyco is not loadable quite yet... */
    DeviceNames[3] = __C2GSMALLOC(".ttyco");
    InstallDriver(kp->userID,3,&ConsoleHeader);

    ttys = fopen("9/etc/tty.config","r");
    while (!feof(ttys)) {
        fgets(line1,80,ttys);
        if (strlen(line1) < 2) continue;    /* skip blank lines   */
        if (line1[0] == '#') continue;       /* skip comment lines */
        sscanf(line,"%s %d %s", filename.text, &devNum, devname);
	filename.length = strlen(filename.text);
#ifdef DEBUG_DRIVER_LOAD
	printf("filename:%s, devNum:%d, devname:%s\n",filename.text,devNum,devname);
#endif
        DeviceNames[devNum] = __C2GSMALLOC(devname);

        /* InitialLoad device file */
	ILuserID = (kp->userID & 0xF0FF) | (((devNum+2) & 0xf) << 8);
	il_rec = InitialLoad2(ILuserID, (Pointer)&filename, 1, 1);
    	if ((e = toolerror())) {
	    printf("Could not load driver: %s, error: %04X\n",filename.text,e);
        } else {
            InstallDriver(ILuserID & 0xF0FF, devNum, il_rec.startAddr);
	    driverUserID[numDrivers++] = ILuserID;
        }
    }
    fclose(ttys);
}

static char *pg = "\pProcyon~GNOME~";
QuitRecGS quitParms;
static PrefixRecGS pr;
static GetRefNumRecGS grn;

int main(int argc, char *argv)
{
extern void NullProcess(void);
extern void test(void);
extern void ROUTINE5(void);
extern CKernData;
extern TEXTTOOLSINFO;
extern void TESTPROC(void);
extern void InitRefnum(void);
extern void AddRefnum(int,int);
extern void init_htable(void);
extern void initPTY(void);
extern int pinit(int);
extern void GetDaMouseMod(void);
extern void InOutStart(void);
extern void InOutEnd(void);
int newPID,stat;
handle fstack;
extern kernTable[];
ResultBuf255Ptr p0;
GSString255Ptr sysRpath;
struct pentry *p;
word state;
byte slot, statereg;
int i;
char *kptr;
GSString255Ptr *pfxRec;
handle emdp;
extern snooperInfo;
word nargs = 0;

	TextStartUp();
        SetInGlobals(0xFF,0x00);
	SetOutGlobals(0xFF,0x00);
	SetErrGlobals(0xFF,0x00);
	SetInputDevice(pascalType,3l);
	SetOutputDevice(pascalType,3l);
	SetErrorDevice(pascalType,3l);
        InitTextDev(input);
        InitTextDev(output);
        InitTextDev(errorOutput);

	kernStatus();
	if (!toolerror()) {
		printf("GNO Kernel already active\n");
		exit(1);
	}
	MessageByName(1,(Pointer)&snooperInfo);
        quitParms.pCount = 0;
        TLStartUp();

        printf("%c\nGNO Kernel v2.0.6 (network)\n",12);
	printf("Copyright 1991-1998, Procyon, Inc.\n%c",6);
	/* initialize kernel queues, etc */
	SetTSPtr(0x8000, 3, (Pointer)kernTable);
	kp = (kernelStructPtr) &CKernData;
#ifdef DEBUG_STARTUP
	printf("\nmain thinks kp is :%08lX\n",kp);
#endif
	kp->userID = userid();
#ifdef DEBUG_GSOS
	kp->gsosDebug = ~0;
#endif

        emdp = NewHandle(0x0100l,kp->userID,0xC005,0l);
        i = _toolErr;
        EMStartUp((word)*emdp,0,0,0,0,0,kp->userID);
#ifdef DEBUG_STARTUP
	printf("After EMStartUp\n");
#endif
        GetDaMouseMod();
        /* mouseMode = ReadMouse(); */
#ifdef DEBUG_STARTUP
	printf("After GetDaMouseMod\n");
#endif

/* start up kernel subsections */
	_seminit();
#ifdef DEBUG_STARTUP
	printf("After _seminit\n");
#endif

	p0 = malloc(sizeof(ResultBuf255));
	p0->bufSize = 255;
	
	procPtr = p = &(kp->procTable[0]);
	p->userID = kp->userID & 0xF0FF;
	p->ttyID = 3;
	p->prefix = malloc(33*sizeof(GSString255Ptr));

        for (i = -1; i < 32; i++) {
            pr.pCount = 2;
	    pr.prefixNum = i;
	    pr.buffer.getPrefix = p0;
	    GetPrefixGS(&pr);
            p->prefix[i+1] = malloc(p0->bufString.length+2);
	    copygsstr(p->prefix[i+1],&p0->bufString);
	}
        nfree(p0);

	p->siginfo = malloc(sizeof(struct sigrec));
	memset(p->siginfo, 0, sizeof(struct sigrec));
	/* allocate the file table with the initial number of files */
        i = sizeof(fdentry)*(FD_SIZE-1);
        p->openFiles = malloc(sizeof(fdtable)+i);
        memset(p->openFiles, 0, sizeof(fdtable)+i);
	p->openFiles->fdTableSize = FD_SIZE;
	p->alarmCount = 0l;

	sysRpath = malloc(sizeof(GSString255));
	strcpy(sysRpath->text,"*:system:system.setup:sys.resources");
	sysRpath->length = strlen(sysRpath->text);
	grn.pCount = 4;
	grn.pathname = sysRpath;
	grn.resNum = 1;
        GetRefNumGS(&grn);
        nfree(sysRpath);
	sysR = grn.refNum;
	if (sysR == 0) {
            printf("FATAL SYSTEM ERROR: Sys.Resources file isn't open!\n"
                   "hit a key to exit\n");
            ReadChar(0);
            exit(1);
        }
        p->openFiles->fdCount = 1;
	p->openFiles->fdLevelMode = 0x8000;
        p->openFiles->fdLevel = 1;
        p->openFiles->fds[sysR-1].refNum = sysR;
	p->openFiles->fds[sysR-1].refLevel = 30;

	InitRefnum();
#ifdef DEBUG_STARTUP
	printf("After InitRefnum\n");
#endif
	AddRefnum(0 /*FDgsos*/, sysR);

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
	p->irq_state = state;
	p->irq_SLTROM = slot;
	p->irq_STATEREG = statereg;
	p->ticks = 0l;
	p->flags = FL_FORKED | FL_COMPLIANT;
	p->parentpid = -1;
	p->LInfo = NULL;
	p->executeHook = NULL;
	p->flpid = 1;
	p->waitq = NULL;
	p->args = "BYTEWRKSNullProcess\0";
	p->alarmCount = 0l;
	p->SANEwap = (word) GetWAP(0,0xA);

        pinit(MAXMSGS);
        init_htable();
        setuptty();         /* initialize the tty subsystem and load drivers */
        initPTY();
#ifdef DEBUG_STARTUP
	printf("After initPTY\n");
#endif

	/* do when console driver is loaded */
	InOutStart();
#ifdef DEBUG_STARTUP
	printf("After InOutStart\n");
#endif

	patchTools();
#ifdef DEBUG_STARTUP
	printf("After patchTools\n");
#endif
	/* Last chance to use a printf() in the kernel's context */
        InitKernel();

	/*
	 * Orca/C 2.0.1 fd based printf()s won't work from this point on in
	 * the kernel's context (NullProcess) because gno doesn't use
	 * fd -1 -2 -3 like Orca does, and fd 1 2 3 aren't open.
	 * printf()s in the tool routines will however work in the context of
	 * the caller's process. Note that a printf() in the kernel in a tool
	 * routine will end up on fd 2 3 of the calling process, NOT the
	 * kernel.  Use kern_printf() inside a tool routine to display
	 * information in the kernel's context (via TextTools).  Make sure
	 * you fflush() any printf()s to stdout before returning to the caller,
	 * or your output could end up buffered and sent to a completely
	 * different process the next time printf() is called.
	 */
	stdin->_file = 1;	/* Attempt to send printf information inside */
	stdout->_file = 2;	/* tool routines to the standard locations   */
	stderr->_file = 3;	/* in the caller's context.                  */

	SendRequest(0x8000,sendToName,(long)pg,0l,NULL);
	/*
	 * Re-setup text tool information for kernel debug output now that
	 * we've patched the hell out of everything.
	 */
        SetInGlobals(0xFF,0x00);
	SetOutGlobals(0xFF,0x00);
	SetErrGlobals(0xFF,0x00);
	SetInputDevice(pascalType,3l);
	SetOutputDevice(pascalType,3l);
	SetErrorDevice(pascalType,3l);
	WriteCString("\n\r\n\r\n\r");

	commonFork(doShell, 1024, 0, NULL, &nargs, &errno);

/*
 * this is the kernel null process. it must NEVER call the assembly
 * _resched routine, or the time slice variable will grow uncontrollably
 * if no other processes are Ready
 */

        NullProcess();
endGNO:
        SendRequest(0x8001,sendToName,(long)pg,0l,NULL);
	unpatchTools();

        /* Shut down the Event Manager */
        EMShutDown();
        DisposeHandle(emdp);

        DeInitKern();

	/* do when console driver is shutdown */
	InOutEnd();

	for (stat = 0; stat < numDrivers; stat++)
		UserShutDown(driverUserID[stat],0);
        TLShutDown();
        HUnlockAll(kp->userID);
        SetPurgeAll(2,kp->userID);
        QuitGS(&quitParms);
}

#include <stdarg.h>
/* turn off debug 25 and on debug 8 for vararg function */
#pragma debug 0
#pragma optimize 79
#define KP_BUFSIZ 256
int kern_printf(const char *format, ...)
{
static char buffer[KP_BUFSIZ]; /* pray this is large enough (need vsnprintf) */
va_list list;
int ret;

	va_start(list, format);
	ret = vsprintf(buffer, format, list);
	if (ret >= KP_BUFSIZ) {
		asm {brk 0xbf}
	}
	WriteCString(buffer);
	va_end(list);
	return ret;
}

