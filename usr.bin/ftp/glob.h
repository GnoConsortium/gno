char        **ftpglob (char *v);
int           letter (char c);
int           digit (char c);
int           any (int c, char *s);
int           blklen (char **av);
char        **blkcpy (char **oav, char **bv);
void          blkfree (char **av0);
char        **copyblk (char **v);
int           gethdir (char *home);
