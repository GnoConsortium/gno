/*	$Id: kvm.h,v 1.1 1998/02/02 08:18:32 taubert Exp $ */

/*
         Kernel VM access routines
         written June 4, 1991 by Jawaid Bazyar
         mod. October 17, 1991 for new call paradigm
         Copyright 1991-1998 Procyon, Inc.

         Note that there is absolutely no VM involved here, I simply kept the
         same names as BSD/SunOS for compatibility.
*/

#ifndef __KVM_H__
#define __KVM_H__

#include "proc.h"

struct kvmt {
    int procIndex;     /* don't futz with this */
    int pid;           /* you can use this to determine pid */
    struct pentry kvm_pent;
};
typedef struct kvmt kvmt;

kvmt *kvm_open(void);
int kvm_close(kvmt *k);
struct pentry *kvmgetproc(kvmt *kd, int pid);
struct pentry *kvmnextproc(kvmt *kd);
int kvmsetproc(kvmt *kd);

#endif /* __KVM_H__ */

