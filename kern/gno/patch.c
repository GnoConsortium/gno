/*	$Id: patch.c,v 1.1 1998/02/02 08:18:40 taubert Exp $ */

segment "KERN2     ";

#pragma optimize 79

#include "proc.h"
#include "gno.h"
#include "sys.h"
#include "sem.h"
#include "/lang/orca/libraries/orcacdefs/stdio.h"
#include "/lang/orca/libraries/orcacdefs/stdlib.h"
#include <memory.h>
#include <gsos.h>
#include <event.h>
#include <locator.h>

struct patchEntry {
    word toolNum;
    longword oldFunc,newFunc;
    longword *chainPoint;
};

/* tool patch entry points */
extern byte NULLTOOLFUNC;
extern byte OURSYSFAILMGR;
extern byte SaveAllPatch;
extern byte RestAllPatch;
extern byte QDSTARTUPPATCH;
extern byte SANESUPATCH;
extern byte SANESDPATCH;
extern byte SANESTPATCH;
extern byte TDISPOSEHAND;
extern byte NewGetNextEvent, NewEMStartUp, NewGetOSEvent, NewOSEventAvail,
            NewEMShutDown,NewEventAvail;

/* tool patch 'old' vectors, for patches that want to call the old routine */
extern longword OLDSAVEALL, OLDRESTALL, OLDQDSTARTUP, OLDSANESU, OLDDISPHAND;
extern longword OldGetNextEvent, OldOSEventAvail, OldGetOSEvent, OldEventAvail;

static struct patchEntry patchArray[] = {
   { 0x0B05, 0l, (longword) (&SaveAllPatch),	&OLDSAVEALL		},
   { 0x0C05, 0l, (longword) (&RestAllPatch),	&OLDRESTALL		},
   { 0x0204, 0l, (longword) (&QDSTARTUPPATCH),	&OLDQDSTARTUP		},
   { 0x020A, 0l, (longword) (&SANESUPATCH),	&OLDSANESU		},
   { 0x0201, 0l, (longword) (&NULLTOOLFUNC),	NULL			},
   { 0x0301, 0l, (longword) (&NULLTOOLFUNC),	NULL			},
   { 0x1503, 0l, (longword) (&OURSYSFAILMGR),	NULL			},
   { 0x030A, 0l, (longword) (&SANESDPATCH),	NULL			},
   { 0x060A, 0l, (longword) (&SANESTPATCH),	NULL			},
/* { 0x1002, 0l, (longword) (&TDISPOSEHAND),	&OLDDISPHAND		}, */
   { 0x0206, 0l, (longword) (&NewEMStartUp),	NULL			},
   { 0x0A06, 0l, (longword) (&NewGetNextEvent),	&OldGetNextEvent	},
   { 0x0B06, 0l, (longword) (&NewEventAvail),	&OldEventAvail		},
   { 0x1606, 0l, (longword) (&NewGetOSEvent),	&OldGetOSEvent		},
   { 0x1706, 0l, (longword) (&NewOSEventAvail),	&OldOSEventAvail	},
   { 0x0306, 0l, (longword) (&NewEMShutDown),	NULL			},
   { ~0, 0l, 0l, NULL}};

void patchTools(void)
{
longword *TLfunc;
int i;
word Tool, Func;

    i = 0;
    while ((Tool = patchArray[i].toolNum) != ~0) {
	Func = (Tool & 0xFF00) >> 8;
	Tool &= 0xff;
	TLfunc = (longword *) GetTSPtr(0x0000, Tool);
	patchArray[i].oldFunc = TLfunc[Func];
	if (patchArray[i].chainPoint) {
	    if (*(patchArray[i].chainPoint)) {
		/* takes care of jmp >$000000 patches */
		*(patchArray[i].chainPoint) |= ((TLfunc[Func]+1) << 8);
	    } else {
		/* takes care of dc i4'0' patches */
		*(patchArray[i].chainPoint) = TLfunc[Func]+1;
	    }
	}
#ifdef DEBUG_TOOL_PATCH
	printf("%lx %lx %lx %lx %lx\n", &TLfunc[Func], patchArray[i].oldFunc,
		patchArray[i].chainPoint, *(patchArray[i].chainPoint),
		patchArray[i].newFunc);
#endif
	TLfunc[Func] = patchArray[i].newFunc-1;
#ifdef DEBUG_TOOL_PATCH
	printf("  %lx\n", TLfunc[Func]);
#endif
	i++;
    }
}

void unpatchTools(void)
{
longword *TLfunc;
int i;
word Tool, Func;

    i = 0;
    while ((Tool = patchArray[i].toolNum) != -1) {
	Func = (Tool & 0xFF00) >> 8;
	Tool &= 0xff;
	TLfunc = (longword *) GetTSPtr(0x0000, Tool);
	TLfunc[Func] = patchArray[i].oldFunc;
	i++;
    }
}

