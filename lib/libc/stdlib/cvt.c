/*
 * Floating point conversion routines.
 * Devin Reade, 1997.
 *
 * $Id: cvt.c,v 1.2 1997/09/21 06:20:37 gdr Exp $
 *
 * This file is formatted with tab stops every 8 columns.
 */

#ifdef __ORCAC__
segment "libc_stdlb";
#endif

#include <stdio.h>
#include <string.h>
#include <sane.h>
#include <math.h>

char *
ecvt (double number, size_t ndigits, int *decpt, int *sign)
{
	static DecForm convert;
	static Decimal d;

	convert.style = FLOATDECIMAL;
	if (ndigits > SIGDIGLEN) {
		ndigits = SIGDIGLEN;
	}
	convert.digits = ndigits;
	s_num2dec(&convert, number, &d);
	*decpt = (int)d.sig.length + d.exp;
	*sign = d.sgn;
	*(d.sig.text + (int)d.sig.length) = '\0'; /* depends on d.sig.unused */
        return d.sig.text;
}

char *
fcvt (double number, size_t ndigits, int *decpt, int *sign)
{
	static DecForm convert;
	static Decimal d;

	convert.style = FIXEDDECIMAL;
	if (ndigits > SIGDIGLEN) {
		ndigits = SIGDIGLEN;
	}
	convert.digits = ndigits;
	s_num2dec(&convert, number, &d);
	*decpt = (int)d.sig.length + d.exp;
	*sign = d.sgn;
	*(d.sig.text + (int)d.sig.length) = '\0'; /* depends on d.sig.unused */
        return d.sig.text;
}

 /*	__dtoa:	These are the comments from the BSD implementation of
		this routine.  Whether or not we have achieved compatibility
		remains to be seen.

	Arguments ndigits, decpt, sign are similar to those
	of ecvt and fcvt; trailing zeros are suppressed from
	the returned string.  If not null, *rve is set to point
	to the end of the return value.  If d is +-Infinity or NaN,
	then *decpt is set to 9999.

	mode:
		0 ==> shortest string that yields d when read in
			and rounded to nearest.
		1 ==> like 0, but with Steele & White stopping rule;
			e.g. with IEEE P754 arithmetic , mode 0 gives
			1e23 whereas mode 1 gives 9.999999999999999e22.
		2 ==> max(1,ndigits) significant digits.  This gives a
			return value similar to that of ecvt, except
			that trailing zeros are suppressed.
		3 ==> through ndigits past the decimal point.  This
			gives a return value similar to that from fcvt,
			except that trailing zeros are suppressed, and
			ndigits can be negative.
		4-9 should give the same return values as 2-3, i.e.,
			4 <= mode <= 9 ==> same return as mode
			2 + (mode & 1).  These modes are mainly for
			debugging; often they run slower but sometimes
			faster than modes 2-3.
		4,5,8,9 ==> left-to-right digit generation.
		6-9 ==> don't try fast floating-point estimate
			(if applicable).

		Values of mode other than 0-9 are treated as mode 0.

		Sufficient space is allocated to the return value
		to hold the suppressed trailing zeros.
	*/

#undef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

char *
__dtoa (double number, int mode, int ndigits, int *decpt, int *sign,
	char **rve)
{
	char *result, *p;

	switch(mode) {
	case 2:
	case 4:
	case 6:
	case 8:
mode0:					/* this label doesn't belong here */
		result = ecvt(number, MAX(1, ndigits), decpt, sign);
		break;                
	case 3:
	case 5:
	case 7:
	case 9:
		result = fcvt(number, ndigits, decpt, sign);
		break;             

	case 0:
	case 1:
	default:
		goto mode0;	/* TEMP KLUDGE */
	}

	/* truncate trailing zeros */
	p = result;
	while (*p) p++;
	p--;
	while ((p > result) && (*p == '0')) {
		*p-- = '\0';
	}
	if (rve != NULL) {
		*rve = p + 1;
	}

	/* set decimal point for NaNs and Infinities */
	if (isnan(number) || isinf(number)) {
		*decpt = 9999;
	}

	return result;
}                                       
