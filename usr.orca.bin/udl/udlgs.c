/*
 * udl - Convert EOL formats freely between MS-DOS (CR/LF), Unix/Amiga (LF),
 *       and Apple (CR).
 *
 * Apple IIgs specific routines.
 *
 * $Id: udlgs.c,v 1.3 1995/02/08 05:15:33 gdr Exp $
 *
 * Copyright (c) 1993-1995 Soenke Behrens, Devin Glyn Reade
 */

#include <orca.h>
#include <shell.h>
#include <gsos.h>
#include <getopt.h>

#define MAIN 1
#include "common.h"

#define QUITFLAG 0x4000 /* udl is restartable */
#define DIRECTORY 0x0F

/*
 * Globals
 */

int theType, theAuxType;    /* Hold type of current file */
Next_WildcardGSPB NextWild; /* for handling ORCA/Shell style wildcards */
Init_WildcardGSPB InitWild;

extern pascal void SystemQuitFlags (unsigned);
extern pascal void SystemQuitPath (GSString255Ptr);
extern int needsgno(void);
extern void begin_stack_check(void);
extern int  end_stack_check(void);

/*
 * Prototypes of functions in this file
 */

extern int CheckGSOSType (char *name);
extern void SetGSOSType (char *name, int type, int auxtype);
extern int right_shell_version (void);

int main(int argc,char *argv[]) {
  FILE *infile, *outfile;
  int Tunix = FALSE;
  int Messy = FALSE;
  int GS = FALSE;
  int careful = FALSE;
  int converted;
  int c;
  static GSString255 gsp;
  static ResultBuf255 rsp;
  int R_flag = FALSE;
  char **p;
  
  verbose = FALSE;
  recursionDepth = 0;
  program_name = NULL;
  tempfile = NULL;
  current_file = NULL;
  NextWild.pathName = NULL;
  in_buffer = out_buffer = NULL;
  pathSlotsUsed = 0;
  pathSlots = 0;
  pathList = NULL;
  *currentDirectory = '\0';
  recursionDepth=0;
  
#ifdef CHECK_STACK
  begin_stack_check();
#endif
  
  /* In case of exit(), free the mem I allocated */
  atexit (cleanup);
  
  /* Make udl restartable */
  SystemQuitFlags (QUITFLAG);
  SystemQuitPath (NULL);
  
  if (right_shell_version() == FALSE) {
    fprintf(stderr,"%s requires at least ORCA/Shell 2.0"
            " or GNO/ME 1.0\n", argv[0]);
    exit (EXIT_FAILURE);
  }

  if ((program_name = strdup (argv[0])) == NULL) {
    fprintf(stderr,"%s: memory allocation failure\n",argv[0]);
    exit (EXIT_FAILURE);
  }

  if (argc < 3) {
    usage();
    exit (EXIT_FAILURE);
  }

  /* Get and process arguments */
  
  optind = 0;
  opterr = 1;
  while ((c = getopt (argc, argv, "pvugmR")) != EOF) {
    switch (c) {
    case 'v':
      verbose = TRUE;
      break;
      
    case 'p':
      careful = TRUE;
      break;
      
    case 'u':
      if (Tunix == TRUE || Messy == TRUE || GS == TRUE) {
        fprintf(stderr, "%s: You may not "
                "specify more than one conversion option\n",program_name);
        exit (EXIT_FAILURE);
      }
      Tunix = TRUE;
      break;
      
    case 'm':
      if (Tunix == TRUE || Messy == TRUE || GS == TRUE) {
        fprintf(stderr, "%s: You may not "
                "specify more than one conversion option\n",program_name);
        exit (EXIT_FAILURE);
      }
      Messy = TRUE;
      break;

    case 'g':
      if (Tunix == TRUE || Messy == TRUE || GS == TRUE) {
        fprintf(stderr, "%s: You may not "
                "specify more than one conversion option\n",program_name);
        exit (EXIT_FAILURE);
      }
      GS = TRUE;
      break;
    
    case 'R':
      R_flag++;
      break;
      
    case '?':              
      usage();
      exit (EXIT_FAILURE);
      
    default:
      fprintf (stderr, "%s: Internal getopt error\n", program_name);
      exit (EXIT_FAILURE);
      break;
    }
  }

  if (optind == argc) { /* no files specified */
    usage();
    exit (EXIT_FAILURE);
  }

  if (Tunix == FALSE && GS == FALSE && Messy == FALSE) {
    fprintf(stderr,"%s: You have to specify a destination "
            "format.\n",program_name);
    exit (EXIT_FAILURE);
  }

  if (verbose == TRUE) {
    printf ("%s version %s\n",program_name,UDL_VERSION);
  }

  if ((in_buffer = malloc(BUFFERSIZE+1)) == NULL ||
      (out_buffer = malloc(BUFFERSIZE+1)) == NULL) {
    fprintf(stderr,"%s: Unable to buffer files\n",program_name);
    exit (EXIT_FAILURE);
  }

  /* Orca Shell: expand wildcards */
  if (!needsgno()) {
    NextWild.pCount = 1;
    InitWild.pCount = 2;
    rsp.bufSize = 259;
    NextWild.pathName = &rsp;
    InitWild.wFile = &gsp;
    if (R_flag) {
      InitWild.flags = 0x2000 | 0x1000;
    } else {
      InitWild.flags = 0;
    }
    
    /* loop through all command line args */
    for (; optind < argc; optind++) {
      size_t i;
      int num_of_files;
      
      i = strlen(argv[optind]);
      strncpy (gsp.text,argv[optind],i);
      gsp.length = i;
      InitWildcardGS (&InitWild);
      num_of_files = 0;
      
      /* loop through all matches of wildcards */
      for (;;) {
        NextWildcardGS (&NextWild);
        if (toolerror()) {
          fprintf(stderr,"%s: Fatal internal error, "
                  "exiting\n", program_name);
          exit (EXIT_FAILURE);
        }
        
        /* No further file found by NextWildcardGS */
        if(!rsp.bufString.length)
          break;
        
        num_of_files++;
        
        if((current_file = calloc(1,rsp.bufString.length + 1)) == NULL) {
          fprintf(stderr,"%s: memory allocation failure\n",program_name);
          exit (EXIT_FAILURE);
        }
        strncpy(current_file, rsp.bufString.text,rsp.bufString.length);
        
        add_to_pathList("",current_file);
        free(current_file);
        current_file = NULL;
      } /* for (;;) */

      if (num_of_files == 0)
        fprintf(stderr,"%s: No files found that match %s\n",
                program_name,argv[optind]);
    } /* for (; optind < argc; optind++) */
  }
  /* gsh or other Gno shell */
  else {
    
    /* save the directory we're in */
    if (getwd(rootdir)==NULL) {
      fprintf(stderr,"%s: Couldn't stat .\n",program_name);
      exit (EXIT_FAILURE);
    }

    for (; optind<argc; optind++) {
      /* set the directory separator character. */
      dirbrk = (strchr(argv[optind],':')!=NULL) ? ':' : '/';
      build_file_list(argv[optind],R_flag);
      chdir(rootdir);
      *currentDirectory = '\0';
    }
  }

  p = pathList;
  while(*p) {
    current_file = *p;
    
    if (CheckGSOSType (current_file) == FALSE) {
      p++;
      continue;
    }
    if (verbose == TRUE) {
      printf("%s: Working on %s\n",program_name,current_file);
    }

    infile = tryopen(current_file,"rwb");
    tempfile = mktemp(strcat(get_path(current_file), "udltmpXX"));
    outfile = tryopen(tempfile,"wb");
    
    if (careful) {
      converted = TRUE; /* always */
      
      if (GS)
        convert_gs(infile,outfile);
      else if (Tunix)
        convert_tunix(infile,outfile);
      else
        convert_messy(infile,outfile);
    } else {
      if (GS)
        converted = convert_fast_gs(infile,outfile);
      else if (Tunix)
        converted = convert_fast_tunix(infile,outfile);
      else
        converted = convert_fast_messy(infile,outfile);
    }
    
    if (fclose (infile) == EOF || fclose (outfile) == EOF) {
      perror ("closing files");
      exit (EXIT_FAILURE);
    }

    if (converted) { /* Temp file contains converted data */
      if (remove (current_file) != 0) {
        perror ("removing original file");
        exit (EXIT_FAILURE);
      }

      if (rename (tempfile,current_file) != 0) {
	perror ("cannot rename temporary file");
	exit (EXIT_FAILURE);
      }
    } else
      remove (tempfile);

    free (tempfile); tempfile = NULL;
    SetGSOSType (current_file, theType, theAuxType);
    p++;
  } /* end while */

#ifdef CHECK_STACK
  fprintf(stderr,"stack usage: %d bytes\n",end_stack_check());
#endif
  
  return (EXIT_SUCCESS);
}

/*
 * CheckGSOSType() ... check if a file is of type TXT or SRC
 *
 * Inputs:
 *   char *name   Name of file to check
 *
 * Outputs:
 *   int   Boolean, TRUE if file type is TXT or SRC, FALSE otherwise
 */

int CheckGSOSType(char *name) {
#define TXT 0x04
#define SRC 0xB0

  static GSString255 gst;
  static FileInfoRecGS fir = {5};
  size_t i;
  
  i = strlen (name);
  gst.length = i;
  strncpy(gst.text,name,i);
  fir.pathname = &gst;
  
  GetFileInfoGS(&fir);
  
  if (toolerror()) {
    fprintf (stderr,"%s: GS/OS error on %s: 0x%04X\n",
             program_name,name,toolerror());
    exit (EXIT_FAILURE);
  }

  if ((fir.fileType != TXT) && (fir.fileType != SRC)) {
    if (verbose && (fir.fileType != DIRECTORY))
      fprintf(stderr,"%s: %s is not of type TXT or "
              "SRC ... skipping\n",program_name,current_file);
    return (FALSE);
  } else {
    theType = fir.fileType;
    theAuxType = fir.auxType;
    return (TRUE);
  }
}

/*
 * SetGSOSType() ... set file and auxtype of a file.
 *
 * Inputs:
 *   char *name   Name of file to be affected
 *   int type   File type it should be set to
 *   int auxtype   Auxiliary type it should be set to
 *
 * Outputs:
 *   None
 */

void SetGSOSType (char *name, int type, int auxtype) {
  static GSString255 gst;
  static FileInfoRecGS fir = {4, NULL, 0xE3};
  size_t i;
  
  i = strlen (name);
  gst.length = i;
  strncpy(gst.text,name,i);
  fir.pathname = &gst;
  fir.fileType = type;
  fir.auxType = auxtype;
  
  SetFileInfoGS(&fir);
  
  if (toolerror()) {
    fprintf (stderr,"%s: GS/OS error on %s: 0x%04X\n",
             program_name,name,toolerror());
    exit (EXIT_FAILURE);
  }
}

/*
 * right_shell_version() ... check if at least ORCA/Shell 2.0 or
 * GNO/ME 1.0 is active.
 *
 * Inputs:
 *   None
 *
 * Output:
 *   int   Boolean, TRUE if shell is satisfactory, FALSE otherwise
 */

int right_shell_version (void) {
  static VersionPB vpb;
  
  VERSION(&vpb);
  
  if (vpb.version[0] < '2' || strcmp (shellid(),"BYTEWRKS") != 0)
    return FALSE;
  else
    return TRUE;
}

/* End Of File */
