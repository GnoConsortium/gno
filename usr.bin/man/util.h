/*
 * Copyright 1995 by Devin Reade <gdr@myrias.com>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 */

/*
 * from util.c
 */

#define DEFAULT_MANPATH "/usr/man"

char *getManpath(void);
char *Xstrdup(char *oldstr, int line, char *file);
void *Xmalloc(size_t size, int line, char *file);
void *Xrealloc(void *oldptr, size_t size, int line, char *file);
char **addToStringArray(char **oldArray, char *string);
char **makePathArray(char *path);
int ncstrcmp(char *a, char *b);
int ncstrncmp (char *a, char *b, unsigned int count);
char *ncstrstr(char *str, char *substr);
char *newerFile(char *path1, char *path2);
char getcharraw(void);
char *basename (char *path);
char *dirname (const char *path);
                                   

/*
 * from utilgs.c
 */

typedef struct {
   unsigned int type;
   unsigned long int auxtype;
} fileType, fileTypePtr;

fileType *getFileType (char *file);
