/* From: few@autodesk.com (Frank Whaley) */

/*
 *	qsort.c - iterative Quicksort
 */

#pragma noroot

#define DEPTH	20	/*  should be adequate for most sorts  */


	static
swb(a, b, len)
	register char *a;
	register char *b;
	register unsigned len;
{
	register char temp;

        /* Handle the odd-number case */
        if (len & 1) {
		temp = *a;
		*a++ = *b;
		*b++ = temp;
		len--;
        }
	asm {
	    ldy	len
loop:	    cpy	#0
            beq	done
            dey
            dey
            lda	[a],y
            tax
            lda	[b],y
            sta	[a],y
            txa
            sta	[b],y
            bra	loop
        done: }

   /*     while ( len-- )
	{
		temp = *a;
		*a++ = *b;
		*b++ = temp;
	} */
}


	int
nqsort(bas, n, wid, cmp)
	char *bas;	/*  base of data		*/
	unsigned n;	/*  number of items to sort	*/
	unsigned wid;	/*  width of an element		*/
	int (*cmp)();	/*  key comparison function	*/
{
	unsigned mumble;
	unsigned j;
	unsigned k;
	unsigned pvt;
	unsigned cnt;
	unsigned lo[DEPTH];
	unsigned hi[DEPTH];

	if ( n < 2 )
		return 0;

	/*  init  */
	cnt = 1;
	lo[0] = 0;
	hi[0] = n - 1;

	while ( cnt-- )
	{
		pvt = lo[cnt];
		j = pvt + 1;
		n = k = hi[cnt];
		while ( j < k )
		{
			while ( (j < k) &&
			        (*cmp)(bas + (j * wid), bas + (pvt * wid)) < 1 )
				++j;

			while ( (j <= k) &&
			        (*cmp)(bas + (pvt * wid), bas + (k * wid)) < 1)
				--k;

			if ( j < k )
				swb(bas + (j++ * wid), bas + (k-- * wid), wid);
			}

		if ( (*cmp)(bas + (pvt * wid), bas + (k * wid)) > 0 )
			swb(bas + (pvt * wid), bas + (k * wid), wid);

		if ( k > pvt )
			--k;

		if ( (k > pvt) && (n > j) && ((k - pvt) < (n - j)) )
		{
			mumble = k;
			k = n;
			n = mumble;
			mumble = pvt;
			pvt = j;
			j = mumble;
		}

		if ( k > pvt )
		{
			lo[cnt] = pvt;
			hi[cnt++] = k;
		}

		if ( n > j )
		{
			lo[cnt] = j;
			hi[cnt++] = n;
		}

		if ( cnt >= DEPTH )
			return -1;
	}

	return 0;
}

/*  END of qsort.c  */

#if 0
short int in[] = {10, 32, -1, 567, 3, 18, 1, -51, 789, 0};

int compr(short *a, short *b)
{
     if (*a < *b) return -1;
     else if (*a > *b) return 1;
     else return 0;
}

int compr1(short *a, short *b)
{
     if (*a < *b) return 1;
     else if (*a > *b) return -1;
     else return 0;
}

int main()
{
unsigned i;

    nqsort(in,10,2,compr1);
    for (i = 0; i < 10; i++) printf("%d\n",in[i]);
    nqsort(in,10,2,compr);
    for (i = 0; i < 10; i++) printf("%d\n",in[i]);
}
#endif
