#pragma noroot
#pragma optimize -1
#include <types.h>
#include <shell.h>
#include <stdlib.h>
#include <string.h>

char *getenv(char *name)
{
Get_VarPB gv;
char *vn;
int l;

   l = strlen(name);
   gv.var_name = malloc(l+1);
   gv.var_name[0] = l; memcpy(gv.var_name+1,name,(size_t)l);
   gv.value = malloc(256l);
   GET_VAR(&gv);
   gv.value[gv.value[0]+1] = 0;
   free(gv.var_name);
   if (gv.value[0] == 0) { free(gv.value); return NULL; }
   vn = malloc(gv.value[0]+1);
   memcpy(vn,gv.value+1,gv.value[0]);
   vn[gv.value[0]] = 0;
   free(gv.value);
   return vn;
}

/*void exit(int blah)
{
	rexit(blah);
} */
