/*
 *	Kernel VM access routines
 *	written June 4, 1991 by Jawaid Bazyar
 *	mod. October 17, 1991 for new call paradigm
 *	Copyright 1991-1996 Procyon, Inc.
 *
 *	Note that there is absolutely no VM involved here, I simply kept the
 *	same names as BSD/SunOS for compatibility.
 *
 *	$Id: kvm.h,v 1.1 1997/02/28 04:42:06 gdr Exp $
 */

#ifndef _GNO_KVM_H_
#define _GNO_KVM_H_

#ifndef __TYPES__
#include <types.h>
#endif

#ifndef _GNO_PROC_H_
#include <gno/proc.h>
#endif

struct kvmt {
    int procIndex;     /* don't futz with this */
    int pid;           /* you can use this to determine pid */
    struct pentry kvm_pent;
};
typedef struct kvmt kvmt;

#ifndef _SYS_CDEFS_H_
#include <sys/cdefs.h>
#endif

kvmt *		kvm_open __P((void));
int		kvm_close __P((kvmt *k));
struct pentry *	kvmgetproc __P((kvmt *kd, int pid));
struct pentry *	kvmnextproc __P((kvmt *kd));
int 		kvmsetproc __P((kvmt *kd));

#endif /* _GNO_KVM_H_ */
