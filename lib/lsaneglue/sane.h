/*
 * File: SANE.h
 *
 * Declarations, macros and prototypes for
 * the SANE glue functions in library
 * lsaneglue.
 *
 * Written in 1997 by Soenke Behrens.
 * This code is hereby placed into the Public Domain.
 */

#ifndef __SANE__
#define __SANE__

#ifndef __TYPES__
#include <types.h>
#endif

/* Decimal representation constants */
#define SIGDIGLEN 0x001C
#define DECSTROUTLEN 0x0050

/* IEEE default environment constant */
#define IEEEDEFAULTENV 0x0000
typedef short environment;

/* Decimal formatting styles */
#define FLOATDECIMAL 0x0000
#define FIXEDDECIMAL 0x0001

/* Exceptions */
#define INVALID 0x0001
#define UNDERFLOW 0x0002
#define OVERFLOW 0x0004
#define DIVBYZERO 0x0008
#define INEXACT 0x0010
typedef short exception;

/* Ordering relations */
#define GREATERTHAN 0
#define LESSTHAN 1
#define EQUALTO 2
#define UNORDERED 3
typedef short relop;

/* Inquiry classes */
#define SNAN 0
#define QNAN 1
#define INFINITE 2
#define ZERONUM 3
#define NORMALNUM 4
#define DENORMALNUM 5
typedef short numclass;

/* Environmental control */

/* Rounding directions */
#define TONEAREST 0
#define UPWARD 1
#define DOWNWARD 2
#define TOWARDZERO 3
typedef short rounddir;

/* Rounding precisions */
#define EXTPRECISION 0
#define DBLPRECISION 1
#define FLOATPRECISION 2
typedef short roundpre;

/* NAN codes */
#define NANSQRT 1       /* Invalid square root such as sqrt(-1)   */
#define NANADD 2        /* Invalid addition such as +INF - +INF   */
#define NANDIV 4        /* Invalid division such as 0/0           */
#define NANMUL 8        /* Invalid multiply such as 0 * INF       */
#define NANREM 9        /* Invalid rem or mod such as x REM 0     */
#define NANASCBIN 17    /* Conversion of invalid ASCII string     */
#define NANCOMP 20      /* Comp NaN converted to floating         */
#define NANZERO 21      /* Attempt to create a NaN with zero code */
#define NANTRIG 33      /* Invalid argument to trig routine       */
#define NANINVTRIG 34   /* Invalid arg to inverse trig routine    */
#define NANLOG 36       /* Invalid argument to log routine        */
#define NANPOWER 37     /* Invalid argument to x^i or x^y routine */
#define NANFINAN 38     /* Invalid argument to financial function */

typedef struct decimal
{
   short sgn;             /* sign 0 for +, 1 for - */
   short exp;             /* decimal exponent */
   struct
   {
      unsigned char length, text[SIGDIGLEN], unused;
   }sig;                  /* significant digits */
} decimal, Decimal;

typedef struct decform
{
   short style;           /* FLOATDECIMAL or FIXEDDECIMAL  */
   short digits;
} decform, DecForm;

typedef void (*haltvector)(void);

/* SANE types are:
 * float    -- IEEE single precision
 * double   -- IEEE double precision
 * extended -- IEEE extended precision
 * comp     -- SANE comp type
 * Decimal  -- SANE decimal string
 * DecForm  -- Controls formatting of decimal strings
  */

/* Function declarations */

void		s_num2dec(DecForm *, extended, Decimal *);
extended	s_dec2num(Decimal *);
void		s_str2dec(char *, short *, Decimal *, short *);
void		s_dec2str(DecForm *, Decimal *, char *);
extended	s_fabs(extended);
extended	s_fneg(extended);
extended	s_remainder(extended, extended, short *);
extended	s_sqrt(extended);
extended	s_rint(extended);
extended	s_scalb(short, extended);
extended	s_logb(extended);
extended	s_copysign(extended, extended);
extended	s_nextfloat(extended, extended);
extended	s_nextdouble(extended, extended);
extended	s_nextextended(extended, extended);
extended	s_log2(extended);
extended	s_log(extended);
extended	s_log1(extended);
extended	s_exp2(extended);
extended	s_exp(extended);
extended	s_exp1(extended);
extended	s_power(extended, extended);
extended	s_ipower(extended, short);
extended	s_compound(extended, extended);
extended	s_annuity(extended, extended);
extended	s_tan(extended);
extended	s_sin(extended);
extended	s_cos(extended);
extended	s_atan(extended);
extended	s_randomx(extended *);
numclass	s_classfloat(extended);
numclass	s_classdouble(extended);
numclass	s_classcomp(extended);
numclass	s_classextended(extended);
long		s_signnum(extended);
void		s_setexception(exception, long);
long		s_testexception(exception);
void		s_sethalt(exception, long);
long		s_testhalt(exception);
void		s_setround(rounddir);
rounddir	s_getround(void);
void		s_setprecision(roundpre);
roundpre	s_getprecision(void);
void		s_setenvironment(environment);
void		s_getenvironment(environment *);
void		s_procentry(environment *);
void		s_procexit(environment);
haltvector	s_gethaltvector(void);
void		s_sethaltvector(haltvector);
relop		s_relation(extended, extended);
extended	s_nan(unsigned char);
extended	s_inf(void);
extended	s_pi(void);

/* SANE tool calls */

extern pascal void SANEBootInit(void) inline(0x010A,dispatcher);
extern pascal void SANEStartUp(Word) inline(0x020A,dispatcher);
extern pascal void SANEShutDown(void) inline(0x030A,dispatcher);
extern pascal Word SANEVersion(void) inline(0x040A,dispatcher);
extern pascal void SANEReset(void) inline(0x050A,dispatcher);
extern pascal Boolean SANEStatus(void) inline(0x060A,dispatcher);
extern pascal void SANEFP816(Word, ...) inline(0x090A,dispatcher);
extern pascal void SANEDecStr816(Word, ...) inline(0x0A0A,dispatcher);
extern pascal void SANEElems816(Word, ...) inline(0x0B0A,dispatcher);

/* FPCP find routine, not part of SANE tool set, found in lsaneglue */
int findfpcp(void); /* Returns slot number of FPCP card or -1 if not found */

#endif
