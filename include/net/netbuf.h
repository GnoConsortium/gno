/* 
 * Copyright (C) 1990 by NeXT, Inc., All Rights Reserved
 *
 */

/* 
 * Network Buffer API (for kernel use only)
 *
 * HISTORY
 * 09-Apr-90  Bradley Taylor (btaylor) at NeXT, Inc.
 *	Created. 
 */
#ifndef _NETBUF_
#define _NETBUF_

typedef struct { char opaque[1]; } *netbuf_t;

#ifdef KERNEL

extern char *nb_map(netbuf_t nb);
extern netbuf_t nb_alloc(unsigned size);
extern netbuf_t nb_alloc_wrapper(void *data, unsigned size,
				 void freefunc(void *), void *freefunc_arg);

extern void nb_free(netbuf_t nb);
extern void nb_free_wrapper(netbuf_t nb);
extern unsigned nb_size(netbuf_t nb);
extern int nb_read(netbuf_t nb, unsigned offset, unsigned size, void *target);
extern int nb_write(netbuf_t nb, unsigned offset, unsigned size, void *source);
extern int nb_shrink_top(netbuf_t nb, unsigned size);
extern int nb_grow_top(netbuf_t nb, unsigned size);
extern int nb_shrink_bot(netbuf_t nb, unsigned size);
extern int nb_grow_bot(netbuf_t nb, unsigned size);

#endif /* KERNEL */
#endif /* _NETBUF_ */
