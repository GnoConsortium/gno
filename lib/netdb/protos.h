segment "netdb     ";

#include <sys/types.h>
#include <stdio.h>

/* herror.c */
void herror(const char *s);

/* rcmd.c */
int rresvport(int *alport);
int _validuser(FILE *hostf, char *rhost, char *luser, char *ruser, int baselen);

/* res_comp.c */
int dn_comp(const u_char *exp_dn, u_char *comp_dn, int length,
	u_char **dnptrs, u_char **lastdnptr);
int dn_expand(const u_char *msg, const u_char *eomorig, const u_char *comp_dn,
	u_char *exp_dn, int length);
void _res_close(void);
int __dn_skipname(const u_char *comp_dn, const u_char *eom);
void __putshort(register u_short s, register u_char *msgp);
void __putlong(register u_long l, register u_char *msgp);

/* res_debug.c */
void __p_query(char *msg);

/* res_init.c */
int res_init(void);

/* res_mkquery.c */
int res_mkquery(int op, const char *dname, int class, int type,
	const char *data, int datalen, const struct rrec *newrr,
	char *buf, int buflen);

/* res_query.c */
int res_query(char *name, int class, int type, u_char *answer, int anslen);
int res_querydomain(char *name, char *domain, int class, int type,
	u_char *answer, int anslen);
int res_search(char *name, int class, int type, u_char *answer, int anslen);

/* res_send.c */
int res_send(const char *buf, int buflen, char *answer, int anslen);
