/*	$Id: net.c,v 1.1 1998/02/02 08:18:36 taubert Exp $ */

/*
 *	GNO/ME Network Support
 *
 *	Copyright 1994-1998, Procyon Enterprises Inc.
 *
 *	Written by Derek Taubert and Jawaid Bazyar
 *
 * KERNInstallNetDriver(inf *netcore,int *ERRNO)
 * KERNsocket(int domain, int type, int protocol, int *ERRNO)
 * KERNbind(int fd, struct sockaddr *my_addr, int addrlen, int *ERRNO)
 * KERNconnect(int fd, struct sockaddr *serv_addr, int addrlen, int *ERRNO)
 * KERNlisten(int fd, int backlog, int *ERRNO)
 * KERNaccept(int fd, struct sockaddr *rem_addr, int *addrlen, int *ERRNO)
 * KERNrecvfrom(int fd, void *buf, size_t len, unsigned int flags, struct sockaddr *rem_addr, int *addrlen, int *ERRNO)
 * KERNsendto(int fd, void *buf, size_t len, unsigned int flags, struct sockaddr *rem_addr, int addrlen, int *ERRNO)
 * KERNrecv(int fd, void *buf, size_t len, unsigned int flags, int *ERRNO)
 * KERNsend(int fd, void *buf, size_t len, unsigned int flags, int *ERRNO)
 *
 * KERNgetpeername(int s, struct sockaddr *peer_addr, int *addrlen, int *ERRNO)
 * KERNgetsockname(int s, struct sockaddr *sock_addr, int *addrlen, int *ERRNO)
 * KERNgetsockopt(int s, int level, int optname, void *optval, int *optlen, int *ERRNO)
 * KERNsetsockopt(int s, int level, int optname, void *optval, int optlen, int *ERRNO)
 */

#include "gno.h"
#include "proc.h"
#include "sys.h"
#include "kernel.h"
#include "net.h"
#include "/lang/orca/libraries/orcacdefs/string.h"
#include "/lang/orca/libraries/orcacdefs/stdlib.h"
#include <locator.h>
#include <orca.h>
#include <memory.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

/*#define DEBUG
#include <debug.h>*/

#pragma optimize 79
segment "KERN3     ";

extern void selwakeup(int col_flag, int pid);

int (*pr_usrreq)(int socknum, int req, void *m, size_t *m_len,
		struct sockaddr *addr, int *addrlen, void *rights) = NULL;

#pragma databank 1
int SOCKioctl(void *dataptr, longword tioc, int sock)
{
	if (!pr_usrreq) return EPFNOSUPPORT;
	return (*pr_usrreq)(sock,PRU_CONTROL,(void *)&tioc,NULL,(struct sockaddr *)dataptr,NULL,NULL);
}

/* socket number to select on, and flags */
int SOCKselect(int pid, int fl, int sock)
{
	if (!pr_usrreq) return EPFNOSUPPORT;
	sock++;	/* downfall of optimization in calling routine */
	(*pr_usrreq)(sock,PRU_SELECT,NULL,(size_t *)&pid,(struct sockaddr *)&fl,NULL,NULL);
	return (fl != 0) ? 1 : 0;
}

int SOCKclose(int sock)
/* Called from inside GS/OS, has to return special error codes */
{
int err = 0;

        if (!pr_usrreq) return EPFNOSUPPORT|0x4300;
        (*pr_usrreq)(
		sock,
		PRU_DISCONNECT,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);
	err = (*pr_usrreq)(
		sock,
		PRU_DETACH,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);
	return (!err) ? 0 : 0x4300 | err;
}

/* returns the refNum field of a file descriptor if that fd is a socket,
   and is valid. Otherwise, returns -ERRNO */

int getsocknum(int fd)
{
fdentryPtr fdp;
extern fdentryPtr getFDptr(int);

    if (!pr_usrreq) return -EPFNOSUPPORT;
    fdp = getFDptr(fd);
    if (fdp == NULL) return -EBADF;
    if (fdp->refType != rtSOCKET) return -ENOTSOCK; /* Not a socket */
    return fdp->refNum;
}

struct rwPBlock {
    word ref;
    void *buffer;
    size_t reqCount;
    size_t xferCount;
    word cachePriority;
};

int SOCKrdwr(struct rwPBlock *pb, word cmd, int sock)
/* Called from inside GS/OS, has to return special error codes */
{
size_t len;
int err;

    if (!pr_usrreq) return 0x4300|EPFNOSUPPORT;
    len = pb->reqCount;
    err = (*pr_usrreq) (
		sock,
		(((cmd & 0xFF) == 0x12) ? PRU_RCVD : PRU_SEND),
		pb->buffer,
		&len,
		NULL,
		NULL,
		NULL);
    pb->xferCount = len;
    return (err == 0) ? 0 : err | 0x4300;
}

#pragma toolparms 1
pascal int KERNInstallNetDriver(void *netcore,int domain,int *ERRNO)
{
    pr_usrreq = netcore;
    /* don't need domain yet, we only support one type of network */
    return 0;
}

pascal int KERNsocket(int domain, int type, int protocol, int *ERRNO)
{
int fd,sock;
fdentryPtr fdp;
fdtablePtr fdt;

	if ((domain != PF_INET) || (!pr_usrreq)) {
	    *ERRNO = EPFNOSUPPORT;
	    return -1;
	}
	if (*ERRNO = (*pr_usrreq)(
		type,
		PRU_ATTACH,
		&sock,
		(size_t *)selwakeup,
		(struct sockaddr *) &protocol,
		NULL,
		NULL)) return -1;
        disableps();
        fdt = PROC->openFiles;
        fdt->fdCount++;

        fdp = allocFD(&fd);
        fdp->refNum = sock;
        fdp->refType = rtSOCKET;
        fdp->refFlags = 0;
        fdp->refLevel = fdt->fdLevel | fdt->fdLevelMode;
        fdp->NLenableMask = 0;
        fdp->NLnumChars = 0;
        fdp->NLtable = NULL;
	AddRefnum(rtSOCKET,sock);
        enableps();
        return fd;
}

pascal int KERNbind(int fd, struct sockaddr *my_addr, int addrlen, int *ERRNO)
{
int err,sock;

        if ((sock = getsocknum(fd)) < 0) {
            *ERRNO = -sock;
            return -1;
        }
	if (*ERRNO = (*pr_usrreq)(
		sock,
		PRU_BIND,
		NULL,
		NULL,
		my_addr,
		&addrlen,
		NULL)) return -1;
	return 0;
}

pascal int KERNconnect(int fd, struct sockaddr *serv_addr, int addrlen, int *ERRNO)
{
int sock;

        if ((sock = getsocknum(fd)) < 0) {
            *ERRNO = -sock;
            return -1;
        }
	if (*ERRNO = (*pr_usrreq)(
		sock,
		PRU_CONNECT,
		NULL,
		NULL,
		serv_addr,
		&addrlen,
		NULL)) return -1;
	return 0;
}

pascal int KERNlisten(int fd, int backlog, int *ERRNO)
{
int sock;

        if ((sock = getsocknum(fd)) < 0) {
            *ERRNO = -sock;
            return -1;
        }
	if (*ERRNO = (*pr_usrreq)(
		sock,
		PRU_LISTEN,
		&backlog,
		NULL,
		NULL,
		NULL,
		NULL)) return -1;
	return 0;
}

pascal int KERNaccept(int fd, struct sockaddr *rem_addr, int *addrlen, int *ERRNO)
{
int newfd,sock;
fdentryPtr fdp;
fdtablePtr fdt;

        if ((sock = getsocknum(fd)) < 0) {
            *ERRNO = -sock;
            return -1;
        }
	if (*ERRNO = (*pr_usrreq)(
		sock,
		PRU_ACCEPT,
		&newfd,
		(size_t *)selwakeup,
		rem_addr,
		addrlen,
		NULL)) return -1;
        disableps();
        fdt = PROC->openFiles;
        fdt->fdCount++;
        fdp = allocFD(&fd);
        fdp->refNum = newfd;
        fdp->refType = rtSOCKET;
        fdp->refFlags = 0;
        fdp->refLevel = fdt->fdLevel | fdt->fdLevelMode;
        fdp->NLenableMask = 0;
        fdp->NLnumChars = 0;
        fdp->NLtable = NULL;
	AddRefnum(rtSOCKET,newfd);
        enableps();
	return fd;
}

pascal int KERNrecvfrom(int fd, void *buf, size_t len, unsigned int flags, struct sockaddr *rem_addr, int *addrlen, int *ERRNO)
{
int sock;
        if ((sock = getsocknum(fd)) < 0) {
            *ERRNO = -sock;
            return -1;
        }
	if (*ERRNO = (*pr_usrreq)(
		sock,
		(flags & MSG_OOB) ? PRU_RCVOOB : PRU_RCVD,
		buf,
		&len,
		rem_addr,
		addrlen,
		NULL)) return -1;
	return len;
}

pascal int KERNsendto(int fd, void *buf, size_t len, unsigned int flags, struct sockaddr *rem_addr, int addrlen, int *ERRNO)
{
int sock;
        if ((sock = getsocknum(fd)) < 0) {
            *ERRNO = -sock;
            return -1;
        }
	if (*ERRNO = (*pr_usrreq)(
		sock,
		(flags & MSG_OOB) ? PRU_SENDOOB : PRU_SEND,
		buf,
		&len,
		rem_addr,
		&addrlen,
		NULL)) return -1;
	return len;
}

pascal int KERNrecv(int fd, void *buf, size_t len, unsigned int flags, int *ERRNO)
{
int sock;
        if ((sock = getsocknum(fd)) < 0) {
            *ERRNO = -sock;
            return -1;
        }
	if (*ERRNO = (*pr_usrreq)(
		sock,
		(flags & MSG_OOB) ? PRU_RCVOOB : PRU_RCVD,
		buf,
		&len,
		NULL,
		NULL,
		NULL)) return -1;
	return len;
}

pascal int KERNsend(int fd, void *buf, size_t len, unsigned int flags, int *ERRNO)
{
int sock;
        if ((sock = getsocknum(fd)) < 0) {
            *ERRNO = -sock;
            return -1;
        }
	if (*ERRNO = (*pr_usrreq)(
		sock,
		(flags & MSG_OOB) ? PRU_SENDOOB : PRU_SEND,
		buf,
		&len,
		NULL,
		NULL,
		NULL)) return -1;
	return len;
}

pascal int KERNgetpeername(int s, struct sockaddr *peer_addr, int *addrlen, int *ERRNO) {
int sock;
        if ((sock = getsocknum(s)) < 0) {
            *ERRNO = -sock;
            return -1;
        }
	if (*ERRNO = (*pr_usrreq)(
		sock,
		PRU_PEERADDR,
		NULL,
		NULL,
		peer_addr,
		addrlen,
		NULL)) return -1;
	return 0;
}

pascal int KERNgetsockname(int s, struct sockaddr *sock_addr, int *addrlen, int *ERRNO) {
int sock;
        if ((sock = getsocknum(s)) < 0) {
            *ERRNO = -sock;
            return -1;
        }
	if (*ERRNO = (*pr_usrreq)(
		sock,
		PRU_SOCKADDR,
		NULL,
		NULL,
		sock_addr,
		addrlen,
		NULL)) return -1;
	return 0;
}

pascal int KERNgetsockopt(int s, int level, int optname, void *optval, int *optlen, int *ERRNO) {
int sock;
        if ((sock = getsocknum(s)) < 0) {
            *ERRNO = -sock;
            return -1;
        }
	if (*ERRNO = (*pr_usrreq)(
		sock,
		PRU_CO_GETOPT,
		&level,
		(size_t *)&optname,
		(struct sockaddr *)optval,
		optlen,
		NULL)) return -1;
	return 0;
}

pascal int KERNsetsockopt(int s, int level, int optname, void *optval, int optlen, int *ERRNO) {
int sock;
        if ((sock = getsocknum(s)) < 0) {
            *ERRNO = -sock;
            return -1;
        }
	if (*ERRNO = (*pr_usrreq)(
		sock,
		PRU_CO_SETOPT,
		&level,
		(size_t *)&optname,
		(struct sockaddr *)optval,
		&optlen,
		NULL)) return -1;
	return 0;
}

pascal int KERNshutdown(int s, int how, int *ERRNO)
{
int sock;

        if ((sock = getsocknum(s)) < 0) {
            *ERRNO = -sock;
            return -1;
        }
	if (*ERRNO = (*pr_usrreq)(
		sock,
		PRU_SHUTDOWN,
		&how,
		NULL,
		NULL,
		NULL,
		NULL)) return -1;
	return 0;
}
#pragma toolparms 0
#pragma databank 0

