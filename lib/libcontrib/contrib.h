/*
 * $Id: contrib.h,v 1.2 1997/10/30 04:57:24 gdr Exp $
 */

#ifndef _GNO_CONTRIB_H_
#define _GNO_CONTRIB_H_

#ifndef __TYPES__
#include <types.h>
#endif

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

#ifndef _SYS_CDEFS_H_
#include <sys/cdefs.h>
#endif

/*
 * Memory allocation routines
 */

void *	LC_xmalloc __P((size_t));
void *	LC_xrealloc __P((void *, size_t));
char *	LC_xstrdup __P((const char *));

/*
 * String Array Functions.
 */

typedef struct LC_StringArrayElem_t {
  char **	lc_vec;
  int		lc_alloced;
  int		lc_used;
} LC_StringArrayElem_t, *LC_StringArray_t;

LC_StringArray_t	LC_StringArrayNew __P((void));
void			LC_StringArrayAdd __P((LC_StringArray_t, char *));
void			LC_StringArrayDelete __P((LC_StringArray_t, char *));
void			LC_StringArrayDestroy __P((LC_StringArray_t));
void			LC_StringArrayClear __P((LC_StringArray_t));
char *			LC_StringArrayCat __P((LC_StringArray_t, int));

/*
 * File copy operations
 */

#define LC_COPY_DATA	0x0001	/* copy the data fork? */
#define LC_COPY_REZ	0x0002	/* copy the resource fork? */
#define LC_COPY_KEEPBUF	0x0004	/* keep the internally allocated buffer? */
#define LC_COPY_BACKUP	0x0008	/* set the backup bit? */

#define LC_COPY_FORKS_MASK	0x0003

GSStringPtr	LC_CopyFileGS __P((GSStringPtr from, GSStringPtr to,
				   unsigned short flags));
char *		LC_CopyFile __P((const char *, const char *, unsigned short));

/*
 * Filename expansion
 */

GSStringPtr	LC_ExpandPathGS __P((GSStringPtr path));
char *		LC_ExpandPath __P((char *path));

#endif	/* _GNO_CONTRIB_H_ */
