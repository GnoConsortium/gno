/*
 * machine/endian.h for the IIgs.
 *
 * $Id: endian.h,v 1.1 1997/02/28 04:42:07 gdr Exp $
 */

#ifndef _MACHINE_ENDIAN_H_
#define _MACHINE_ENDIAN_H_ 1

/*
 * Define the order of 32-bit words in 64-bit words.
 *
 * These may cause problems under Orca/C as there is no base 64 bit
 * type.
 */
#define	_QUAD_HIGHWORD 1
#define	_QUAD_LOWWORD 0

/*
 * Definitions for byte order, according to byte significance from low
 * address to high.
 */
#define	LITTLE_ENDIAN	1234	/* LSB first: i386, vax, 65c816 */
#define	BIG_ENDIAN	4321	/* MSB first: 68000, ibm, net */
#define	PDP_ENDIAN	3412	/* LSB first in word, MSW first in long */

#define	BYTE_ORDER	LITTLE_ENDIAN

#define ntohl(x)	((0xff & (x)>>24) | (0xff00 & (x)>>8) | \
			(0xff0000 & (x)<<8) | (0xff000000 & (x)<<24))
#define ntohs(x)	((0xff & (x)>>8) | (0xff00 & (x)<<8))
#define htonl(x)	ntohl(x)
#define htons(x)	ntohs(x)

#define NTOHL(x)	ntohl(x)
#define	NTOHS(x)	ntohs(x)
#define	HTONL(x)	htonl(x)
#define	HTONS(x)	htons(x)

#endif /* _MACHINE_ENDIAN_H_ */
