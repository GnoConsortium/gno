#pragma stacksize 2048
#pragma debug 24

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <gsos.h>
#include <shell.h>
#include <orca.h>

#pragma lint -1
/* #define DEBUG_MODE */

#define version "Make - Version 1.1"
#define NAME_SIZE 80           /* max file name size */
#define MSG_SIZE 80            /* max size of any generated message */
#define PARAM_SIZE 256         /* max size of any input parameter */
#define d_flag 'D'             /* display date/time info */
#define p_flag 'P'             /* programmer (debug) mode */
#define s_flag 'S'             /* silent mode */
#define target_delim ':'       /* target file delimiters */
#define dep_delims " ,"        /* dependant file delimiters */
#define comment_char '#'       /* start of comment */

/* System error return values */

#define BAD_OPTION     1
#define NO_INPUT_FILE  2
#define NO_MEM_AVAIL   3
#define SYNTAX_ERR     4
#define USER_ABORT     5
#define SHELL_ERR      6
#define EOF_ERR        7
#define ILLEGAL_FILE  8

#define MEM_UNAVAIL "Not enough memory available to execute."

typedef struct readVariableDCB {
   Str255 *varName, *value;
} readVariableDCB;

typedef struct executeDCB {
   int flag;
   char *commandString;
} executeDCB;

/* Routine prototypes */

void ShowLastParam(void);
int GetInfo(FileInfoRecPtrGS file_info, int params, char *file_name);
int FileExists(char *);
void ErrorMessage(char *);
void ErrorAbort(char *, int);
int CheckDepFiles(char *dep_params, char *target_file);
int GetTargetFile(char *params, char *target);
int ExecuteMakeCommand(char *make_command, char *target_file);

enum MODES {PARAM, CONTINUATION, COMMAND, DONE};

unsigned char valid_types[] = { 0x04, 0xB0, 0 };   /* valid file types TXT, SRC */

FILE *make_file;
char msg[MSG_SIZE], make_param[PARAM_SIZE], prog_name[NAME_SIZE];
int silent, debugging, disp_date_time;

int main(int argc, char *argv[])
{
   int i, type_ok;
   enum MODES mode;
   char *tmp_file, *make_file_name;
   char first_char, *dep_params, *target_file;
   FileInfoRecGS file_info;
   StopPB stop_info;
/*
   Initialize all global variables so that we can run as a re-startable
   command in the shell environment.
*/
   make_param[0] = '\0';           /* init to NULL string */
 /*dep_params[0] = '\0';
   target_file[0] = '\0'; */	/* NONONONONONONO */
   msg[0] = '\0';
   strcpy(prog_name, argv[0]);     /* for error message processing */
   silent = FALSE;
   debugging = FALSE;
   disp_date_time = FALSE;

/* Allocate some dynamic work variables */

   if ( (make_file_name = malloc(NAME_SIZE)) == NULL )
       ErrorAbort(MEM_UNAVAIL, NO_MEM_AVAIL);
   if ( (dep_params = malloc(PARAM_SIZE)) == NULL )
       ErrorAbort(MEM_UNAVAIL, NO_MEM_AVAIL);
   if ( (target_file = malloc(NAME_SIZE)) == NULL )
       ErrorAbort(MEM_UNAVAIL, NO_MEM_AVAIL);
   *make_file_name = 0; /* this was a bug- jb */

/* Parse the command line information */

   for (i = 1; i < argc; i++)
       if ( argv[i][0] == '-' || argv[i][0] == '+' ) { /* command line option */
           switch (toupper(argv[i][1])) {
               case p_flag: debugging = TRUE;   /* set appropriate flag */
                            break;
               case s_flag: silent = TRUE;
                            break;
               case d_flag: disp_date_time = TRUE;
                            break;
               default:     ErrorAbort("Invalid option specified.", BAD_OPTION);
           }
       }
       else
           strcpy(make_file_name, argv[i]);    /* not an opt, must be a file */

   if ( !silent )
       puts(version);      /* display version info */

   if ( strlen(make_file_name) == 0 ) {    /* did we get an input file name */
         strcpy(make_file_name, "makefile");
/*       fputs("MAKE file: ", stdout);       /* nope, ask for one */
/*       if ( gets(make_file_name) == NULL ) /* make sure we got one */
/*           exit(NO_INPUT_FILE);            /* if not, give up */
   }

/*
   Allocate some work variables:
   tmp_file - will be used to check for a file with the default extention.
*/

   if ( (tmp_file = malloc(NAME_SIZE)) == NULL )
       exit(NO_MEM_AVAIL);

   strcpy(tmp_file, make_file_name);   /* get a copy of the specified name */
   strcat(tmp_file, ".make");          /* default extention */

   if ( FileExists(tmp_file) )             /* check for default name */
       strcpy(make_file_name, tmp_file);   /* got it */
   else
       if ( !FileExists(make_file_name) ) {    /* nope check specified name */
           sprintf(msg, "Make file %s does not exist.", make_file_name);
           ErrorAbort(msg, NO_INPUT_FILE);     /* still not there, give up */
       }

   free(tmp_file);     /* done with this memory */

   GetInfo(&file_info, 3, make_file_name);     /* Check input file type */
   type_ok = FALSE;
   for ( i=0; i < sizeof(valid_types); i++ )   /* search valid types */
       if ( file_info.fileType == valid_types[i] ) {
           type_ok = TRUE;
           break;
       }

   if ( !type_ok )
       ErrorAbort("Input file must be either TXT or SRC.", ILLEGAL_FILE);

   /* Open the input file */

   if ( (make_file = fopen(make_file_name, "r")) == NULL ) {
       perror(prog_name);
       sprintf(msg, "Error opening input file %s.", make_file_name);
       ErrorAbort(msg, NO_INPUT_FILE);
   }

   mode = PARAM;           /* initial mode is parameter search */

   /* Read the input file and process the make commands */

   while ( fgets(make_param, sizeof(make_param), make_file) != NULL ) {
       if ( (i = strpos(make_param, '\n')) >= 0 )
           make_param[i] = '\0';   /* remove NL character */

       if ( debugging )
           printf("make_param:[%s] i: %d, mode: %d\n", make_param, i, mode);

       first_char = make_param[0]; /* we will check this char often */

       if ( (first_char == '\0') || (first_char == comment_char) ) {
           mode = PARAM;           /* blank line terminates COMMAND/CONTINUE mode */
           continue;               /* ignore blank lines & comment lines */
       }

       if ( mode == CONTINUATION ) {   /* CheckDepFiles wants more params */
           strcpy(dep_params, make_param);     /* pre-load params */
           mode = CheckDepFiles(dep_params, target_file);
       }
       else if ( first_char != ' ' ) {
           mode = PARAM;               /* back into param search mode */
           strcpy(dep_params, make_param); /* set up for parsing */
           if ( GetTargetFile(dep_params, target_file) ) /* we should find one */
               mode = CheckDepFiles(dep_params, target_file);
           else
               ErrorAbort("Target file specification error, no ';' found.",SYNTAX_ERR);
       }
       else if ( mode == COMMAND )
           mode = ExecuteMakeCommand(make_param, target_file);

       STOP(&stop_info);           /* Check for user termination */
       if ( stop_info.stop == 1 )
           ErrorAbort("User termination.", USER_ABORT);
  }   /* while reading parameters and not in DONE mode */

   if ( mode == CONTINUATION )
       ErrorAbort("Unexpected End-Of-File encountered after continuation.", EOF_ERR);

   fclose(make_file);
   exit(0);
}

void ErrorMessage(char *error_mesg)
{
   printf("%s: %s\n", prog_name, error_mesg);
}

void ShowLastParam(void)
{
   ErrorMessage("Error occured at:");
   ErrorMessage(make_param);
}

void ErrorAbort(char *abort_mesg, int error_num)
{
   ErrorPB error_params;

   if ( (error_params.error = toolerror()) != 0 ) /* If a tool error */
       ERROR(&error_params);               /* Have the shell display the msg */
   ErrorMessage(abort_mesg);               /* Display my message */
   if ( strlen(make_param) > 0 )           /* If param parsing */
       ShowLastParam();                    /*     display the param */
   exit(error_num);
}

GSString255Ptr strcpygs(GSString255Ptr gs_str, char *str)
{
   gs_str->length = strlen(str);
   strcpy((char *) gs_str->text, str);
   return(gs_str);
}

int GetInfo(FileInfoRecPtrGS file_info, int params, char *file_name)
{
   GSString255 file_info_name;

#ifdef DEBUG_MODE
   puts("GetInfo");
#endif

   file_info->pCount = params;
   file_info->pathname = strcpygs(&file_info_name, file_name);
   GetFileInfoGS(file_info);
   return(toolerror());
}

int FileExists(char *file_name)
{
   FileInfoRecGS file_info;

   return(GetInfo(&file_info, 2, file_name) ? FALSE : TRUE);
}

void TrimLeft(char *str)
{
   char *tmp;

   tmp = str;
   while ( *tmp == ' ' && *tmp != '\0' ) tmp++;
   strcpy(str, tmp);
}

void TrimRight(char *str)
{
   int tmp;

   tmp = strlen(str);
   while ( tmp >= 0 && str[tmp] == ' ') tmp--;
   str[++tmp] = '\0';
}

int ExecuteMakeCommand(char *make_command, char *target_file)
{
   char var_name[80], var_value[80];
   Get_VarPB get_var;
   executeDCB exec_params;

#ifdef DEBUG_MODE
   puts("ExecuteMakeCommand()");
#endif

   TrimLeft(make_command);

   if ( !silent )
       printf("\n%s\n\n", make_command);

   make_command[strlen(make_command)+1] = '\0';  /* add an extra NULL */
   exec_params.flag = 1;       /* use existing variable table */
   exec_params.commandString = make_command;

   EXECUTE(&exec_params);

   strcpy((char *) var_name, "\pStatus");
   get_var.var_name = var_name;
   get_var.value = var_value;

   GET_VAR(&get_var);
   if ( toolerror() != 0 )
       ErrorAbort("Error occured during READ_VARIABLE.", SHELL_ERR);

   var_value[var_value[0]+1] = '\0';
   if ( debugging )
       printf("var_name: %p = %p\n",var_name, var_value);

   if ( strcmp((char *) var_value+1, "0") != 0 ) {
       remove(target_file);
       ErrorAbort("Error occurred during the last command.", SHELL_ERR);
   }   /* Status != "0" */

   return COMMAND;     /* remain in COMMAND mode */
}

int GetTargetFile(char *params, char *target)
{
   int ch;

#ifdef DEBUG_MODE
   puts("GetTargetFile()");
#endif

   if ( (ch = strpos(params, target_delim)) == -1 )
       return FALSE;

   strncpy(target, params, ch);
   target[ch] = '\0';
   TrimLeft(target);
   strcpy(params, &params[ch+1]);

   if ( debugging )
       printf("params: [%s] target: [%s]\n", params, target);

   return TRUE;
}

void GetModDate(TimeRec *info, char *date)
{
   sprintf(date, "%d/%02d/%02d", info->month+1, info->day+1, info->year);
}

void GetModTime(TimeRec *info, char *time)
{
   sprintf(time, "%d:%02d:%02d", info->hour, info->minute, info->second);
}

int DepFileOlder(char *target_file, char *dep_file)
{
   FileInfoRecGS target, dependant;
   char mod_date[12];

#ifdef DEBUG_MODE
   puts("DepFileOlder()");
   printf("target_file:[%s] dep_file:[%s]\n", target_file, dep_file);
#endif

   if ( GetInfo(&target, 7, target_file) != 0 ) {
       ErrorMessage("target file does not exist.");
       return TRUE;        /* if any errors, assume target not found */
   }

   if ( debugging || disp_date_time ) { /* Diaplay mod date if in debug mode */
       GetModDate(&target.modDateTime, mod_date);
       printf("[Date]Target: %s = %s\n", target_file, mod_date);
   }

   if ( GetInfo(&dependant, 7, dep_file) != 0 ) {
       ErrorMessage("dependant file does not exist.");
       return FALSE;       /* dependant file exist? No, user must be nuts */
   }

   if ( debugging || disp_date_time ) { /* Diaplay mod date if in debug mode */
       GetModDate(&dependant.modDateTime, mod_date);
       printf("[Date]Dependant: %s = %s\n", dep_file, mod_date);
   }

   /* Both the target file and the dependant file exists, check mod info */

   if ( ( dependant.modDateTime.year == target.modDateTime.year ) &&
        ( dependant.modDateTime.month == target.modDateTime.month ) &&
        ( dependant.modDateTime.day == target.modDateTime.day ) ) { /* equal dates */

       if ( debugging || disp_date_time ) { /* display debug info */
           GetModTime(&target.modDateTime, mod_date);
           printf("[Time]Target: %s = %s\n", target_file, mod_date);
           GetModTime(&dependant.modDateTime, mod_date);
           printf("[Time]Dependant: %s = %s\n", dep_file, mod_date);
       }
       /* check times */
       if ( dependant.modDateTime.hour > target.modDateTime.hour )
           return TRUE;
       else if ( dependant.modDateTime.hour < target.modDateTime.hour )
           return FALSE;
       else if ( dependant.modDateTime.minute > target.modDateTime.minute)
           return TRUE;
       else if ( dependant.modDateTime.minute < target.modDateTime.minute)
           return FALSE;
       else if ( dependant.modDateTime.second > target.modDateTime.second)
           return TRUE;                        /* dependant older */
       else
           return FALSE;                       /* target older */
   } else                                      /* unequal dates */
       if ( dependant.modDateTime.year > target.modDateTime.year )
           return TRUE;
       else if  ( dependant.modDateTime.year < target.modDateTime.year )
           return FALSE;
       else if ( dependant.modDateTime.month > target.modDateTime.month )
           return TRUE;
       else if ( dependant.modDateTime.month < target.modDateTime.month )
           return FALSE;
       else if ( dependant.modDateTime.day > target.modDateTime.day )
           return TRUE;                        /* dependant is older */
       else
           return FALSE;                       /* target is older */
}

void FlushParams(void)
{
   char *param_token;
   int read_file = FALSE, flush_complete = FALSE;

#ifdef DEBUG_MODE
   puts("FlushParams()");
#endif

   do {
       if ( read_file ) { /* handle continuation, read another line */
           if ( fgets(make_param, sizeof(make_param), make_file) == NULL )
               ErrorAbort("Unexpected End-Of-File encountered after continuation.", EOF_ERR);
           param_token = strtok(make_param, dep_delims); /* init parser */
           read_file = FALSE;                       /* reset read flag */
       }
       else
           param_token = strtok(NULL, dep_delims); /* continue reading */

       if ( debugging )
           printf("param_token:[%s]\n", param_token);

       if ( param_token == NULL || *param_token == comment_char )   /* done yet? */
           flush_complete = TRUE;                          /* yes, signal end */
       else if ( *param_token == '\\' )    /* continuation? */
           read_file = TRUE;               /* yes, read another line */
   } while ( !flush_complete );
}

int CheckDepFiles(char *dep_params, char *target_file)
{
   int first_file;
   char *dep_file;

#ifdef DEBUG_MODE
   puts("CheckDepFiles()");
   printf("dep_params: [%s] target_file: [%s]\n",dep_params, target_file);
#endif

   first_file = TRUE;

   do {        /* parse all dependant files from dep_params */
       if ( first_file ) { /* first time through, init parser */
           dep_file = strtok(dep_params, dep_delims); /* get 1st file name */
           if ( dep_file == NULL ) /* there must be at least 1 file name */
               ErrorAbort("No dependant files specified.", SYNTAX_ERR);
           first_file = FALSE;
       }
       else    /* all subsequent parsing is done from the previous params */
           dep_file = strtok(NULL, dep_delims);

       if ( dep_file == NULL )     /* any more parameters? */
           return PARAM;           /* nope, back to param search mode */

       if ( debugging )
           printf("dep_file:[%s]\n", dep_file);

       if ( *dep_file == '\\' )     /* continuation? */
           return CONTINUATION;        /* yes, get the next line */
       else if ( *dep_file == comment_char ) /* trailing comment? */
           return PARAM;               /* yes, skip all commands until next param */
       else if ( *dep_file == '\0' )   /* Null string? */
           continue;                       /* yes, look some more */
       else if ( DepFileOlder(target_file, dep_file) ) {
           FlushParams();          /* skip over any continuation stuff */
           return COMMAND;         /* dependant is older, rebuild target */
       }
   } while ( dep_file != NULL );  /* until no more dependant files */

   return PARAM;   /* return to param search mode */
}
