void          intr           (int sig, int code);
void          lostpeer       (int sig, int code);
int           cmdscanner     (int top);
struct cmd   *getcmd         (char *name);
void          makeargv       (void);
char         *slurpstring    (void);
void          help           (int argc, char **argv);
