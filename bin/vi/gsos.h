/*
 * Apple //gs GSOS Machine-dependent routines. 
 */

void flushbuf(void);

int  inchar(void);
int  outchar(char);
void outstr(char *);
void toutstr(char *);
void beep(void);
void windinit(void);
void windexit(int);
void windgoto(int,int);
void delay(void);
unsigned int sleep(unsigned int);
