/*
 * $Id: contrib.h,v 1.1 1997/10/03 04:49:40 gdr Exp $
 */

#ifndef _CONTRIB_H_
#define _CONTRIB_H_

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
void			LC_StringArrayClear __P((LC_StringArray_t));
char *			LC_StringArrayCat __P((LC_StringArray_t, int));

#endif	/* _CONTRIB_H_ */
