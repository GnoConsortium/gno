	case	on
	mcopy	saneglue.mac

*
* Provide SANE glue code for functions declared in <sane.h>
*
* Supported functions at this time:
*    s_num2dec
*    s_dec2num
*    s_str2dec
*    s_dec2str
*    s_fabs
*    s_fneg
*    s_remainder
*    s_sqrt
*    s_rint
*    s_scalb
*    s_logb
*    s_copysign
*    s_nextfloat
*    s_nextdouble
*    s_nextextended
*    s_log2
*    s_log
*    s_log1
*    s_exp2
*    s_exp
*    s_exp1
*    s_power
*    s_ipower
*    s_compound
*    s_annuity
*    s_tan
*    s_sin
*    s_cos
*    s_atan
*    s_randomx
*    s_classfloat
*    s_classdouble
*    s_classcomp
*    s_classextended
*    s_signnum
*    s_setexception
*    s_testexception
*    s_sethalt
*    s_testhalt
*    s_setround
*    s_getround
*    s_setprecision
*    s_getprecision
*    s_setenvironment
*    s_getenvironment
*    s_procentry
*    s_procexit
*    s_gethaltvector
*    s_sethaltvector
*    s_relation
*    s_nan
*    s_inf
*    s_pi
*
* Written in 1997 by Soenke Behrens.
* This code is hereby placed into the Public Domain.
*

*
* Dummy function to take care of saneglue.root, which
* can then be discarded.
*

dummy	start
	copy 13:ainclude:e16.sane ; Apple-supplied SANE EQUs
	end

****************************************************************
*
*  void s_num2dec (DecForm *f, extended x, Decimal *d);
*
*  Convert SANE extended to SANE decimal record
*
*  See also: Apple Numerics Manual, pg 26ff 
*
****************************************************************
*

s_num2dec start

	csub	(4:decf_p,10:ext_x,4:dec_p)

	ph4	decf_p
	phptr	ext_x
	ph4	dec_p
	fx2dec
	sterr

	ret
	end

****************************************************************
*
*  extended s_dec2num (Decimal *d);
*
*  Convert SANE decimal record to SANE extended
*
*  See also: Apple Numerics Manual, pg 26ff 
*
****************************************************************
*

s_dec2num start
	using	sane_tmp
	csub	(4:dec_p)

	ph4	dec_p
	ph4	#ext_tmp
	fdec2x
	sterr

	ret	10:ext_tmp
	end

****************************************************************
*
*  void s_str2dec (char *s, short *index, Decimal *d, short *validPrefix);
*
*  Convert SANE decimal string (C-style) to SANE decimal record
*
*  See also: Apple Numerics Manual, pg 30f
*
****************************************************************
*

s_str2dec start

	csub	(4:str_p,4:idx_p,4:dec_p,4:bool_p)

	ph4	str_p
	ph4	idx_p
	ph4	dec_p
	ph4	bool_p
	fcstr2dec
	sterr

	ret
	end

****************************************************************
*
*  void s_dec2str (DecForm *f, Decimal *d, char *s);
*
*  Convert SANE decimal record to SANE decimal string (C-style)
*
*  See also: Apple Numerics Manual, pg 31ff
*
****************************************************************
*

s_dec2str start
str_l	equ	1
space	equ	str_l+2

	csub	(4:decf_p,4:dec_p,4:str_p),space

	ph4	decf_p
	ph4	dec_p
	ph4	str_p
	fdec2str
	sterr

* Now convert the P-string pointed to by str_p into a C-string
* Get length of string
	stz	str_l
	short	m
	lda	[str_p]
	sta	str_l
* If string is empty, don't try to copy
	bne	lab1
	bra	break
* Move string backwards one byte
lab1	ldy	#0
loop	iny
	lda	[str_p],y
	dey
	sta	[str_p],y
	iny
	cpy	str_l
	bne	loop
* Terminate string with 0
break	lda	#0
	ldy	str_l
	sta	[str_p],y
	long	m

	ret
	end

***************************************************************
*
*  extended s_fabs (extended x);
*
*  Return absolute value of x
*
*  See also: Apple Numerics Manual, pg 49 
*
****************************************************************
*

s_fabs	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	fabsx
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

***************************************************************
*
*  extended s_fneg (extended x);
*
*  Return negated value of x
*
*  See also: Apple Numerics Manual, pg 49 
*
****************************************************************
*

s_fneg	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	fnegx
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_remainder (extended x, extended y, short *quo);
*
*  Compute remainder of x and y (x rem y)
*
*  See also: Apple Numerics Manual, pg 46f 
*
****************************************************************
*

s_remainder start
	using	sane_tmp
	csub	(10:ext_x,10:ext_y,4:quo_p)

	phptr	ext_y
	phptr	ext_x
	fremx
	sterr
	txa		; store 7 low-order bits of magnitude of
	sta	[quo_p] ; integer quotient
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_sqrt (extended x);
*
*  Compute square root of x
*
*  See also: Apple Numerics Manual, pg 46 
*
****************************************************************
*

s_sqrt	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	fsqrtx
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_rint (extended x);
*
*  Round x to integral value
*
*  See also: Apple Numerics Manual, pg 46f 
*
****************************************************************
*

s_rint	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	frintx
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_scalb (short n, extended x);
*
*  Scale binary exponent, result = x * 2^n
*
*  See also: Apple Numerics Manual, pg 50
*
****************************************************************
*

s_scalb	start
	using	sane_tmp
	csub	(2:n,10:ext_x)

	ph2	n
	phptr	ext_x
	fscalbx
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_logb (extended x);
*
*  Compute binary exponent of normalized x
*
*  See also: Apple Numerics Manual, pg 50
*
****************************************************************
*

s_logb	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	flogbx
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_copysign (extended x, extended y);
*
*  Return y with sign of x
*
*  See also: Apple Numerics Manual, pg 49
*
****************************************************************
*

s_copysign start
	using	sane_tmp
	csub	(10:ext_x,10:ext_y)

	phptr	ext_x
	phptr	ext_y
	fcpysgnx
	sterr
	copyx	ext_y,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_nextfloat (extended x, extended y);
*
*  Return next float number after (float) x in direction of
*  (float) y
*
*  See also: Apple Numerics Manual, pg 50
*
****************************************************************
*

s_nextfloat start
	using	sane_tmp
	csub	(10:ext_x,10:ext_y)

* Convert extended parameters to single
	phptr	ext_x
	ph4	#sgl_x
	fx2s
	sterr
	phptr	ext_y
	ph4	#sgl_y
	fx2s
	sterr
* Now invoke nextafter function
	ph4	#sgl_y
	ph4	#sgl_x
	fnexts
	sterr
* Convert result back to extended
	ph4	#sgl_x
	ph4	#ext_tmp
	fs2x
	sterr

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_nextdouble (extended x, extended y);
*
*  Return next double number after (double) x in direction of
*  (double) y
*
*  See also: Apple Numerics Manual, pg 50
*
****************************************************************
*

s_nextdouble start
	using	sane_tmp
	csub	(10:ext_x,10:ext_y)

* Convert extended parameters to double
	phptr	ext_x
	ph4	#dbl_x
	fx2d
	sterr
	phptr	ext_y
	ph4	#dbl_y
	fx2d
	sterr
* Now invoke nextafter function
	ph4	#dbl_y
	ph4	#dbl_x
	fnextd
	sterr
* Convert result back to extended
	ph4	#dbl_x
	ph4	#ext_tmp
	fd2x
	sterr

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_nextextended (extended x, extended y);
*
*  Return next extended number after x in direction of y
*
*  See also: Apple Numerics Manual, pg 50
*
****************************************************************
*

s_nextextended start
	using	sane_tmp
	csub	(10:ext_x,10:ext_y)

	phptr	ext_y
	phptr	ext_x
	fnextx
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_log2 (extended x);
*
*  Compute base-2 logarithm of x
*
*  See also: Apple Numerics Manual, pg 62
*
****************************************************************
*

s_log2	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	flog2x
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_log (extended x);
*
*  Compute natural (base-e) logarithm of x
*
*  See also: Apple Numerics Manual, pg 62
*
****************************************************************
*

s_log	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	flnx
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_log1 (extended x);
*
*  Compute natural (base-e) logarithm of (1+x)
*
*  See also: Apple Numerics Manual, pg 62
*
****************************************************************
*

s_log1	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	fln1x
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_exp2 (extended x);
*
*  Compute base-2 exponential of x
*
*  See also: Apple Numerics Manual, pg 63f.
*
****************************************************************
*

s_exp2	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	fexp2x
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_exp (extended x);
*
*  Compute natural (base-e) exponential of x
*
*  See also: Apple Numerics Manual, pg 63f.
*
****************************************************************
*

s_exp	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	fexpx
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_exp1 (extended x);
*
*  Compute the base-e exponential minus 1 (exp(x)-1)
*
*  See also: Apple Numerics Manual, pg 63f.
*
****************************************************************
*

s_exp1	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	fexp1x
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_power (extended x, extended y);
*
*  Compute the general exponential x^y
*
*  See also: Apple Numerics Manual, pg 63f.
*
****************************************************************
*

s_power	start
	using	sane_tmp
	csub	(10:ext_x,10:ext_y)

	phptr	ext_y
	phptr	ext_x
	fxpwry
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_ipower (extended x, short i);
*
*  Compute the integer exponential x^i
*
*  See also: Apple Numerics Manual, pg 63f.
*
****************************************************************
*

s_ipower start
	using	sane_tmp
	csub	(10:ext_x,2:i)

	ph2	i
	phptr	ext_x
	fxpwri
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_compound (extended r, extended n);
*
*  Compute compound (1+r)^n, where r is interest rate and n is
*  periods (may be non-integral)
*
*  See also: Apple Numerics Manual, pg 64f.
*
****************************************************************
*

s_compound start
	using	sane_tmp
	csub	(10:ext_r,10:ext_n)

	phptr	ext_r
	phptr	ext_n
	ph4	#ext_tmp
	fcompound
	sterr

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_annuity (extended r, extended n);
*
*  Compute annuity (1-(1+r)^n)/r, where r is interest rate and
*  n is periods (may be non-integral)
*
*  See also: Apple Numerics Manual, pg 65
*
****************************************************************
*

s_annuity start
	using	sane_tmp
	csub	(10:ext_r,10:ext_n)

	phptr	ext_r
	phptr	ext_n
	ph4	#ext_tmp
	fannuity
	sterr

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_tan (extended x);
*
*  Compute the tangent of x
*
*  See also: Apple Numerics Manual, pg 66f.
*
****************************************************************
*

s_tan	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	ftanx
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_sin (extended x);
*
*  Compute the sine of x
*
*  See also: Apple Numerics Manual, pg 66f.
*
****************************************************************
*

s_sin	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	fsinx
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_cos (extended x);
*
*  Compute the cosine of x
*
*  See also: Apple Numerics Manual, pg 66f.
*
****************************************************************
*

s_cos	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	fcosx
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_atan (extended x);
*
*  Compute the arctangent of x
*
*  See also: Apple Numerics Manual, pg 66f.
*
****************************************************************
*

s_atan	start
	using	sane_tmp
	csub	(10:ext_x)

	phptr	ext_x
	fatanx
	sterr
	copyx	ext_x,ext_tmp

	ret	10:ext_tmp
	end

****************************************************************
*
*  extended s_randomx (extended *x);
*
*  Return next pseudo-random number and update integral x
*
*  See also: Apple Numerics Manual, pg 67
*
****************************************************************
*

s_randomx start
	using	sane_tmp
	csub	(4:ext_p)

	ph4	ext_p
	frandx
	sterr

	ret	4:ext_p
	end

****************************************************************
*
*  numclass s_classfloat (extended x);
*
*  Return classification of (float) x
*
*  See also: Apple Numerics Manual, pg 44
*
****************************************************************
*

s_classfloat start
res	equ	1
space	equ	res+2
	using	sane_tmp
	csub	(10:ext_x),space

* Convert parameter to float
	phptr	ext_x
	ph4	#sgl_x
	fx2s
	sterr
* Call class function
	ph4	#sgl_x
	fclasss
	sterr
* Store result
	short	i	; set high byte to 0
	long	i
	txa
* Convert to C return values
	cmp	#FCSNAN	; Signaling NaN
	bne	lab1
	lda	#$0000
	bra	bye
lab1	cmp	#FCQNAN	; Quiet NaN
	bne	lab2
	lda	#$0001
	bra	bye
lab2	cmp	#FCINF	; Infinity
	bne	lab3
	lda	#$0002
	bra	bye
lab3	cmp	#FCZERO	; Zero
	bne	lab4
	lda	#$0003
	bra	bye
lab4	cmp	#FCNORM	; Normalized
	bne	lab5
	lda	#$0004
	bra	bye
lab5	cmp	#FCDENORM	; Denormalized
	bne	err
	lda	#$0005
	bra	bye
err	lda	#$FFFF	; Unknown return code, return -1
bye	sta	res

	ret	2:res
	end

****************************************************************
*
*  numclass s_classdouble (extended x);
*
*  Return classification of (double) x
*
*  See also: Apple Numerics Manual, pg 44
*
****************************************************************
*

s_classdouble start
res	equ	1
space	equ	res+2
	using	sane_tmp
	csub	(10:ext_x),space

* Convert parameter to double
	phptr	ext_x
	ph4	#dbl_x
	fx2d
	sterr
* Call class function
	ph4	#dbl_x
	fclassd
	sterr
* Store result
	short	i	; set high byte to 0
	long	i
	txa
* Convert to C return values
	cmp	#FCSNAN	; Signaling NaN
	bne	lab1
	lda	#$0000
	bra	bye
lab1	cmp	#FCQNAN	; Quiet NaN
	bne	lab2
	lda	#$0001
	bra	bye
lab2	cmp	#FCINF	; Infinity
	bne	lab3
	lda	#$0002
	bra	bye
lab3	cmp	#FCZERO	; Zero
	bne	lab4
	lda	#$0003
	bra	bye
lab4	cmp	#FCNORM	; Normalized
	bne	lab5
	lda	#$0004
	bra	bye
lab5	cmp	#FCDENORM	; Denormalized
	bne	err
	lda	#$0005
	bra	bye
err	lda	#$FFFF	; Unknown return code, return -1
bye	sta	res

	ret	2:res
	end

****************************************************************
*
*  numclass s_classcomp (extended x);
*
*  Return classification of (comp) x
*
*  See also: Apple Numerics Manual, pg 44
*
****************************************************************
*

s_classcomp start
res	equ	1
space	equ	res+2
	using	sane_tmp
	csub	(10:ext_x),space

* Convert parameter to comp
	phptr	ext_x
	ph4	#cmp_x
	fx2c
	sterr
* Call class function
	ph4	#cmp_x
	fclassc
	sterr
* Store result
	short	i	; set high byte to 0
	long	i
	txa
* Convert to C return values
	cmp	#FCSNAN	; Signaling NaN
	bne	lab1
	lda	#$0000
	bra	bye
lab1	cmp	#FCQNAN	; Quiet NaN
	bne	lab2
	lda	#$0001
	bra	bye
lab2	cmp	#FCINF	; Infinity
	bne	lab3
	lda	#$0002
	bra	bye
lab3	cmp	#FCZERO	; Zero
	bne	lab4
	lda	#$0003
	bra	bye
lab4	cmp	#FCNORM	; Normalized
	bne	lab5
	lda	#$0004
	bra	bye
lab5	cmp	#FCDENORM	; Denormalized
	bne	err
	lda	#$0005
	bra	bye
err	lda	#$FFFF	; Unknown return code, return -1
bye	sta	res

	ret	2:res
	end

****************************************************************
*
*  numclass s_classextended (extended x);
*
*  Return classification of x
*
*  See also: Apple Numerics Manual, pg 44
*
****************************************************************
*

s_classextended start
res	equ	1
space	equ	res+2

	csub	(10:ext_x),space

	phptr	ext_x
	fclassx
	sterr
* Store result
	short	i	; set high byte to 0
	long	i
	txa
* Convert to C return values
	cmp	#FCSNAN	; Signaling NaN
	bne	lab1
	lda	#$0000
	bra	bye
lab1	cmp	#FCQNAN	; Quiet NaN
	bne	lab2
	lda	#$0001
	bra	bye
lab2	cmp	#FCINF	; Infinity
	bne	lab3
	lda	#$0002
	bra	bye
lab3	cmp	#FCZERO	; Zero
	bne	lab4
	lda	#$0003
	bra	bye
lab4	cmp	#FCNORM	; Normalized
	bne	lab5
	lda	#$0004
	bra	bye
lab5	cmp	#FCDENORM	; Denormalized
	bne	err
	lda	#$0005
	bra	bye
err	lda	#$FFFF	; Unknown return code, return -1
bye	sta	res

	ret	2:res
	end

****************************************************************
*
*  long s_signnum (extended x);
*
*  Return sign of x, 0 if positive and 1 if negative
*
*  See also: Apple Numerics Manual, pg 44
*
****************************************************************
*

s_signnum start
res	equ	1
space	equ	res+4

	csub	(10:ext_x),space
	lda	ext_x+8	get sign bit
	bmi	lab1
	stz	res
	bra	lab2
lab1	lda	#1
	sta	res

lab2	stz	res+2
	ret	4:res
	end

* This, the original implementation of s_signnum, had to be
* discarded because of a bug in SANE fclassx. Rather than fix
* fclassx, s_signnum was rewritten.
*
*s_signnum start
*res	equ	1
*space	equ	res+4
*
*	csub	(10:ext_x),space
*
*	phptr	ext_x
*	fclassx
*	sterr
* Store result
*	bpl	plus
*	lda	#1
*	sta	res
*	bra	lab1
*plus	stz	res	
*
*lab1   stz	res+2
*	ret	4:res
*	end

****************************************************************
*
*  void s_setexception (exception e, long b);
*
*  Clears SANE exceptions according to flags in e if b is 0, sets
*  these exceptions otherwise; may cause halt
*
*  See also: Apple Numerics Manual, pg 54ff
*
****************************************************************
*

s_setexception start
	csub	(2:e,4:b)

* As e is passed, the exceptions are in 0-4.
	lda	e
* Just to be extra-cautious, clear all bits but 0-4
	and	#%0000000000011111
	sta	e
* Now check whether to set or clear flags
	lda	b
	ora	b+2	; if b == 0
	bne	lab1	; clear flags, don't set them
	lda	e
	xba		; flags need to be in high word
	eor	#$FFFF	; reverse contents of e
	sta	e
	fgetenv
	sterr
	txa
	and	e	; clear bits indicated by e
	pha
	fsetenv
	sterr
	bra	lab2
lab1	lda	e	; set flags
	pha
	fsetxcp
	sterr

lab2	ret
	end

****************************************************************
*
*  long s_testexception (exception e);
*
*  Return true if any SANE exception indicated by flags in e is
*  set, return false otherwise
*
*  See also: Apple Numerics Manual, pg 54ff
*
****************************************************************
*

s_testexception start
res	equ	1
space	equ	res+4

	csub	(2:e),space

* As e is passed, the exceptions are in 0-4.
	lda	e
* Just to be extra-cautious, clear all bits but 0-4
	and	#%0000000000011111
	sta	e

	ph2	e
	ftestxcp
	beq	lab1	; No exceptions set -> lab1
	sterr
	lda	#1
	sta	res
	bra	lab2
lab1	sterr
	stz	res
lab2	stz	res+2

	ret	4:res
	end

****************************************************************
*
*  void s_sethalt (exception e, long b);
*
*  Clears SANE exception halts according to flags in e if b is 0,
*  sets these halts otherwise
*
*  See also: Apple Numerics Manual, pg 54ff
*
****************************************************************
*

s_sethalt start
	csub	(2:e,4:b)

* Just to be extra-cautious, clear all bits in e that
* don't refer to halts: Everything but bits 0-4
	lda	e
	and	#%0000000000011111
	sta	e
* Now check whether to set or clear flags
	lda	b
	ora	b+2	; if b == 0
	bne	lab1	; clear flags, don't set them
	lda	e
	eor	#$FFFF	; reverse contents of e
	sta	e
	fgetenv
	sterr
	txa
	and	e	; clear bits indicated by e
	pha
	fsetenv
	sterr
	bra	lab2
lab1	fgetenv		; set bits indicated by e
	sterr
	txa
	ora	e
	pha
	fsetenv
	sterr

lab2	ret
	end

****************************************************************
*
*  long s_testhalt (exception e);
*
*  Return true if any SANE exception halt indicated by flags in e
*  is set, return false otherwise
*
*  See also: Apple Numerics Manual, pg 54ff
*
****************************************************************
*

s_testhalt start
res	equ	1
space	equ	res+4

	csub	(2:e),space

* Just to be extra-cautious, clear all bits in e that
* don't refer to halts: Everything but bits 0-4
	lda	e
	and	#%0000000000011111
	sta	e

	fgetenv
	sterr
	txa
	and	e
	beq	lab1	; No halts set -> lab1
	lda	#1
	sta	res
	bra	lab2
lab1	stz	res
lab2	stz	res+2

	ret	4:res
	end

****************************************************************
*
*  void s_setround (rounddir r);
*
*  Set rounding direction to r
*
*  See also: Apple Numerics Manual, pg 52f
*
****************************************************************
*

s_setround start
	csub	(2:r)

* Shift r into bits 14/15
	lda	r	; Bits 0/1
	xba		; 8/9
	asl	a	; 9/10
	asl	a	; 10/11
	asl	a	; 11/12
	asl	a	; 12/13
	asl	a	; 13/14
	asl	a	; 14/15, done
	sta	r

	fgetenv
	sterr
	txa
	and	#%0011111111111111 ; Clear bits 14/15
	ora	r	; Set them according to r
	pha
	fsetenv
	sterr

	ret
	end

****************************************************************
*
*  rounddir s_getround (void);
*
*  Get rounding direction
*
*  See also: Apple Numerics Manual, pg 52f
*
****************************************************************
*

s_getround start
res	equ	1
space	equ	res+2

	csub	,space

	fgetenv
	sterr
	txa
	and	#%1100000000000000  ; Clear everything but bits 14/15
	xba		; Put into 6/7
	lsr	a	; 5/6
	lsr	a	; 4/5
	lsr	a	; 3/4
	lsr	a	; 2/3
	lsr	a	; 1/2
	lsr	a	; 0/1, done
	sta	res

	ret	2:res
	end

****************************************************************
*
*  void s_setprecision (roundpre p);
*
*  Set rounding precision to p
*
*  See also: Apple Numerics Manual, pg 53
*
****************************************************************
*

s_setprecision start
	csub	(2:p)

* Shift p into bits 6/7
	lda	p	; Bits 0/1
	asl	a	; 1/2
	asl	a	; 2/3
	asl	a	; 3/4
	asl	a	; 4/5
	asl	a	; 5/6
	asl	a	; 6/7, done
	sta	p

	fgetenv
	sterr
	txa
	and	#%1111111100111111  ; Clear bits 6/7
	ora	p	; Set them according to p
	pha
	fsetenv
	sterr

	ret
	end

****************************************************************
*
*  roundpre s_getprecision (void);
*
*  Get rounding precision
*
*  See also: Apple Numerics Manual, pg 53
*
****************************************************************
*

s_getprecision start
res	equ	1
space	equ	res+2

	csub	,space

	fgetenv
	sterr
	txa
	and	#%0000000011000000  ; Clear everything but bits 6/7
	lsr	a	; 5/6
	lsr	a	; 4/5
	lsr	a	; 3/4
	lsr	a	; 2/3
	lsr	a	; 1/2
	lsr	a	; 0/1, done
	sta	res

	ret	2:res
	end

****************************************************************
*
*  void s_setenvironment (environment e);
*
*  Set SANE environment word to e
*
*  See also: Apple Numerics Manual, pg 57
*
****************************************************************
*

s_setenvironment start
	csub	(2:e)

	ph2	e
	fsetenv
	sterr

	ret
	end

****************************************************************
*
*  void s_getenvironment (environment *e);
*
*  Get SANE environment word and store it in e
*
*  See also: Apple Numerics Manual, pg 57
*
****************************************************************
*

s_getenvironment start
	csub	(4:eptr)

	fgetenv
	sterr
	txa
	sta	[eptr]

	ret
	end

****************************************************************
*
*  void s_procentry (environment *e);
*
*  Get SANE environment word and store it in e, set SANE
*  environment word to IEEE default (all zero)
*
*  See also: Apple Numerics Manual, pg 57
*
****************************************************************
*

s_procentry start
	csub	(4:eptr)

	ph4	eptr
	fprocentry
	sterr

	ret
	end

****************************************************************
*
*  void s_procexit (environment e);
*
*  Store current exceptions, set SANE environment word to e,
*  signal stored exceptions.
*
*  See also: Apple Numerics Manual, pg 57
*
****************************************************************
*

s_procexit start
	csub	(2:e)

	ph2	e
	fprocexit
	sterr

	ret
	end

****************************************************************
*
*  haltvector s_gethaltvector (void);
*
*  Return SANE halt vector
*
*  See also: Apple Numerics Manual, pg 54
*
****************************************************************
*

s_gethaltvector start
res	equ	1
space	equ	res+4

	csub	,space

	fgethv
	sterr
	stx	res	; low portion of pointer
	tya		; Y contains bytes 2 and 3
	xba		; put 3 into low position in A
	and	#$00FF	; and discard 2
	sta	res+2

	ret	4:res
	end

****************************************************************
*
*  void s_sethaltvector (haltvector v);
*
*  Set SANE halt vector to v
*
*  See also: Apple Numerics Manual, pg 54
*
****************************************************************
*

s_sethaltvector start
	csub	(4:v)

	ph4	v
	fsethv
	sterr

	ret
	end

****************************************************************
*
*  relop s_relation (extended x, extended y);
*
*  Compare x and y, return their relation so that "x Relation y"
*  is true
*
*  See also: Apple Numerics Manual, pg 49
*
****************************************************************
*

s_relation start
res	equ	1
space	equ	res+2

	csub	(10:ext_x,10:ext_y),space

	phptr	ext_x
	phptr	ext_y
	fcmpx
	sterr
	short	i
	txa
	cmp	#$0040	; x > y
	bne	lab1
	lda	#$0000
	bra	bye
lab1	cmp	#$0080	; x < y
	bne	lab2
	lda	#$0001
	bra	bye
lab2	cmp	#$0002	; x == y
	beq	bye
	cmp	#$0001	; x unordered y
	bne	err
	lda	#$0003
	bra	bye
err	lda	#$FFFF	; Unknown return code, return -1
bye	sta	res
	long	i

	ret	2:res
	end

****************************************************************
*
*  extended s_nan (unsigned char c);
*
*  Return a NaN with code c
*
****************************************************************
*

s_nan	start

	csub	(2:c)

	lda	c
	bne	lab1
	lda	#$15
lab1	short	m
	sta	nan_x+6
	long	m

	ret	10:nan_x

nan_x	dc	h'0000000000000040FF7F' ; Hex encoding of a NaN
	end

****************************************************************
*
*  extended s_inf (void);
*
*  Return +INF
*
****************************************************************
*

s_inf	start

	csub

	ret	10:inf_x

inf_x	dc	h'0000000000000000FF7F' ; Hex encoding of +INF
	end

****************************************************************
*
*  extended s_pi (void);
*
*  Return pi constant, which is stored as 3.1415926535897932385
*
****************************************************************
*

s_pi	start

	csub

	ret	10:pi_x

pi_x	dc	h'35C26821A2DA0FC90040' ; Hex encoding of pi
	end

****************************************************************
*
* Common data area for glue code
*
****************************************************************
*

sane_tmp privdata
ext_tmp	dc	e'0'	; Temporary result variable
sgl_x	dc	f'0'	; Float parameter 1
sgl_y	dc	f'0'	; Float parameter 2
dbl_x	dc	d'0'	; Double parameter 1
dbl_y	dc	d'0'	; Double parameter 2
cmp_x	dc	d'0'	; Comp parameter 1
	end

* End Of File
