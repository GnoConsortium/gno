
#include <sys/types.h>
#include <unistd.h>
#include <sys/uio.h>

#include "protos.h"

/* MAJOR HACK ALERT!  This won't work right for packet based sockets! */

size_t writev(int fd, struct iovec *iov, int iovcnt)
{
int i;
size_t size, total_size = 0;

	for (i=0; i<iovcnt; i++) {
		size = write(fd, iov[i].iov_base, iov[i].iov_len);
		total_size += size;
		if (size != iov[i].iov_len) break;
	}
	return total_size;
}

