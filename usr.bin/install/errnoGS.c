/*
 * Copyright 1996 Devin Reade <gdr@myrias.com>.
 * All rights reserved.
 *
 * For copying and distribution information, see the file "COPYING"
 * accompanying this file.
 *
 * $Id: errnoGS.c,v 1.1 1996/03/31 23:38:31 gdr Exp $
 */

#include <gsos.h>
#include <stdarg.h>
#include <stdio.h>
#include "install.h"

#pragma lint -1
#pragma debug 0
#pragma optimize -1

#define NONE    "no error"
#define UNKNOWN "unknown error"

segment "errnoGS___";

typedef struct errEntry {
   unsigned short num;
  char *str;
} errEntry;

static errEntry
sys_errlistGS[] = {
   { badSystemCall,    "bad system call number" },
  { invalidPcount,    "invalid parameter count" },
  { gsosActive,       "GS/OS already active" },
  { devNotFound,      "device not found" },
  { invalidDevNum,    "invalid device number" },
  { drvrBadReq,       "bad request or command" },
  { drvrBadCode,      "bad control or status code" },
  { drvrBadParm,      "bad call parameter" },
  { drvrNotOpen,      "character device not open" },
  { drvrPriorOpen,    "character device already open" },
  { irqTableFull,     "interrupt table full" },
  { drvrNoResrc,      "resources not available" },
  { drvrIOError,      "I/O error" },
  { drvrNoDevice,     "device not connected" },
  { drvrBusy,         "call aborted; driver is busy" },
  { drvrWrtProt,      "device is write protected" },
  { drvrBadCount,     "invalid byte count" },
  { drvrBadBlock,     "invalid block address" },
  { drvrDiskSwitch,   "disk has been switched" },
  { drvrOffLine,      "device off line/ no media present" },
  { badPathSyntax,    "invalid pathname syntax" },
  { tooManyFilesOpen, "too many files open on server volume" },
  { invalidRefNum,    "invalid reference number" },
  { pathNotFound,     "subdirectory does not exist" },
  { volNotFound,      "volume not found" },
  { fileNotFound,     "file not found" },
  { dupPathname,      "create or rename with existing name" },
  { volumeFull,       "volume full error" },
  { volDirFull,       "volume directory full" },
  { badFileFormat,    "version error (incompatible file format)" },
  { badStoreType,     "unsupported (or incorrect) storage type" },
  { eofEncountered,   "end-of-file encountered" },
  { outOfRange,       "position out of range" },
  { invalidAccess,    "access not allowed" },
  { buffTooSmall,     "buffer too small" },
  { fileBusy,         "file is already open" },
  { dirError,         "directory error" },
  { unknownVol,       "unknown volume type" },
  { paramRangeErr,    "parameter out of range" },
  { outOfMem,         "out of memory" },
  { dupVolume,        "duplicate volume name" },
  { notBlockDev,      "not a block device" },
  { invalidLevel,     "specifield level outside legal range" },
  { damagedBitMap,    "block number too large" },
  { badPathNames,     "invalid pathnames for ChangePath" },
  { notSystemFile,    "not an executable file" },
  { osUnsupported,    "Operating System not supported" },
  { stackOverflow,    "too many applications on stack" },
  { dataUnavail,      "data unavailable" },
  { endOfDir,         "end of directory has been reached" },
  { invalidClass,     "invalid FST call class" },
  { resForkNotFound,  "file does not contain required resource" },
  { invalidFSTID,     "error - FST ID is invalid" },
  { invalidFSTop,     "invalid FST operation" },
  { fstCaution,       "FST handled call, but result is weird" },
  { devNameErr,       "device exists with same name as replacement name" },
  { defListFull,      "device list is full" },
  { supListFull,      "supervisor list is full" },
  { fstError,         "generic FST error" },
  { resExistsErr,     "cannot expand file, resource already exists" },
  { resAddErr,        "cannot add resource fork to this type file" },
  { networkError,     "generic network error" },
  { 0,                NONE } /* we shouldn't see this */
};

unsigned short errnoGS = 0;

char *
strerrorGS(unsigned short num)
{
   int i;

  if (num == 0) return NONE;
  i = 0;
  while (sys_errlistGS[i].num) {
      if (sys_errlistGS[i].num == num) {
         return sys_errlistGS[i].str;
     }
      i++;
  }
  return UNKNOWN;
}

void
perrorGS(char *format, ...)
{
   va_list ap;

  va_start(ap,format);
  vfprintf(stderr,format,ap);
  fprintf(stderr,": %s\n",strerrorGS(errnoGS));
  va_end(ap);
  return;
}
