/*
 * $Id: hostname.c,v 1.2 1997/09/21 06:05:00 gdr Exp $
 */

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#include <sys/param.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <orca.h>
#include <locator.h>
#include <memory.h>
#include <misctool.h>

typedef struct NMRec {
	word blockLen;
	unsigned char nameLength;
	char nameString[64];
} NMRec;

typedef union {
	struct {
		word messageID;
		word createFlag;
	} w;
	unsigned long l;
} ResponseR;

int
sethostname(const char *name, int namelen) {
	int size;
	NMRec rec;
	ResponseR mbnResp;

	strcpy(rec.nameString, "Procyon~GNO/ME~HoStNaMe");
	rec.nameLength = 23;
	size = MIN(MAXHOSTNAMELEN-1, namelen);
	strncpy(rec.nameString+23, name, size);
	(rec.nameString+23)[size] = 0;
	rec.blockLen = 2 + (1+23) + (1+size);

	mbnResp.l = MessageByName(1,(Pointer)&rec);
	if (_toolErr) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}

int
gethostname(char *name, int namelen) {
	NMRec rec = {30,23,"Procyon~GNO/ME~HoStNaMe"}; /* why blockLen = 30? */
	ResponseR mbnResp;
	Handle message;

	mbnResp.l = MessageByName(0,(Pointer)&rec);
	if (_toolErr) {
		errno = ENOENT;
		return -1;
	}
#if 0
	printf("MBN returned %X %X\n", mbnResp.w.messageID,
		mbnResp.w.createFlag);
#endif
	MessageCenter(getMessage,mbnResp.w.messageID,
	message = NewHandle(0l,userid(),0,0l));
	if (_toolErr) {
		errno = ENOMEM;
		return -1;
	}
	strncpy(name, (char *)*message+6+2+(1+23),
		MIN(MAXHOSTNAMELEN, namelen));
	DisposeHandle(message);
	return 0;
}
