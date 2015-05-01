/****************************************************************
*
*  setjmp.h - nonlocal jump library
*
*  February 1989
*  Mike Westerfield
*
*  Copyright 1989
*  Byte Works, Inc.
*
****************************************************************/

#ifndef __setjmp__
#define __setjmp__

typedef int jmp_buf[6];

void            longjmp(jmp_buf, int);
int             setjmp(jmp_buf);

#ifndef __KeepNamespacePure__
void            _longjmp(jmp_buf, int);
int             _setjmp(jmp_buf);
#endif

#endif
