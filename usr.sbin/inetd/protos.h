
#include <gno/gno.h>
#include <texttool.h>

#define _exit(__i) return(__i)

segment "inetd     ";

int fork_child(struct servtab *sep, int dofork, int ctrl);
void reapchild(int sig, int code);
void config(int sig, int code);
void retry(int sig, int code);
void setup(struct servtab *sep);
struct servtab *enter(struct servtab *cp);
int setconfig(void);
void endconfig(void);
struct servtab *getconfigent(void);
void freeconfig(struct servtab *cp);
char *skip(char **cpp);
char *nextline(FILE *fd);
char *newstr(char *cp);
void socktitle(char *a, int s);
void echo_stream(int s, struct servtab *sep);
void echo_dg(int s, struct servtab *sep);
void discard_stream(int s, struct servtab *sep);
void discard_dg(int s, struct servtab *sep);
void initring(void);
void chargen_stream(int s, struct servtab *sep);
void chargen_dg(int s, struct servtab *sep);
long machtime(void);
void machtime_stream(int s, struct servtab *sep);
void machtime_dg(int s, struct servtab *sep);
void daytime_stream(int s, struct servtab *sep);
void daytime_dg(int s, struct servtab *sep);
void print_service(char *action, struct servtab *sep);

