/*	$Id: diag.c,v 1.2 1998/02/22 05:05:42 taubert Exp $ */

/* kernel diagnostics and error routines */
#pragma optimize 79

#include "proc.h"
#include "sys.h"
#include "/lang/orca/libraries/orcacdefs/stdio.h"

struct intState {
  word irq_A;
  word irq_X;
  word irq_Y;
  word irq_S;  /* +06 */
  word irq_D;
  byte irq_P;
  byte irq_B;
  byte irq_e;
  byte irq_K;    /* +13 */
  word irq_PC;   /* +14 */
  word dummy[2]; /* +16 */
  word lastTool; /* +20 */
};
extern kernelStructPtr kp;

#pragma databank 1

void traceback(word stack)
{
    word i, j;

    for (j = stack+1; (j<stack+256) && (j<0xC000);) {
	kern_printf("[%04X]:", j);
	for (i=j&0x000f; i; i--) {
	    kern_printf(" --");
	}
	for (i=(j&0xfff0)+0x10; j < i; j++) {
	    kern_printf(" %02X", *((byte *)j));
	}
	kern_printf("\n\r");
    }
}

void PRINTBRK(word stack, struct intState *p)
{
int pid;
struct pentry *pr;

   pr = &(kp->procTable[Kgetpid()]);
   asm {
       tsc
       sta >0x600
   }
   printf("BRK: pid: %d St: %04X\n",pr->flpid,stack);
/* traceback(stack); */
   printf("A:%04X X:%04X Y:%04X S:%04X D:%04X B:%02X P:%02X PC:%02X%04X\n"
          "e:%02X Last Tool: %04X\n",
      p->irq_A, p->irq_X, p->irq_Y, p->irq_S, p->irq_D, p->irq_B, p->irq_P,
      p->irq_K, p->irq_PC,p->irq_e,p->lastTool);
   printf("proc: PC:%02X%04X P:%04X\n",pr->irq_K, pr->irq_PC, pr->irq_P);
}

void PANIC(char *str)
{
    word stack;

    disableps(); /* shut down context switching */
    *((byte *) 0xE0C022l) = 0x1F;
    asm {
	tsc
	sta stack
	lda >0xE0C029
	and #0xFF7F
	sta >0xE0C029
    }
    /*
     * NOTE: When PANIC() is modified, this constant added to stack should
     * be tuned to point to the caller's return address.
     */
    traceback(stack + 0xA);
    kern_printf("SYSTEM PANIC: %s\n\r",str);

noway:
    goto noway;
}

#pragma databank 0

