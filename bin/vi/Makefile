o/alloc.a: alloc.c stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile alloc.c keep=o/alloc

o/CHARSET.a: CHARSET.C stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile CHARSET.C keep=o/CHARSET

o/cmdline.a: cmdline.c stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile cmdline.c keep=o/cmdline

o/DEC.a: DEC.C stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile DEC.C keep=o/DEC

o/EDIT.a: EDIT.C stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile EDIT.C keep=o/EDIT

o/FILEIO.a: FILEIO.C stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile FILEIO.C keep=o/FILEIO

o/FORMAT.L.a: FORMAT.L.C stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile FORMAT.L.C keep=o/FORMAT.L

o/gsos.a: gsos.c gsos.h stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile -p gsos.c keep=o/gsos

o/HELP.a: HELP.C stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile HELP.C keep=o/HELP

o/INC.a: INC.C stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile INC.C keep=o/INC

o/LINEFUNC.a: LINEFUNC.C stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile LINEFUNC.C keep=o/LINEFUNC

o/main.a: main.c stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile main.c keep=o/main

o/MARK.a: MARK.C stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile MARK.C keep=o/MARK

o/misccmds.a: misccmds.c stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile misccmds.c keep=o/misccmds

o/MK.a: MK.C stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile MK.C keep=o/MK

o/PARAM.a: PARAM.C stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile PARAM.C keep=o/PARAM

o/regexp.a: regexp.c env.h regexp.h regmagic.h
  compile regexp.c keep=o/regexp

o/REGSUB.a: REGSUB.C env.h regexp.h regmagic.h
  compile REGSUB.C keep=o/REGSUB

o/S.IO.a: S.IO.C stevie.h env.h ascii.h keymap.h param.h term.h macros.h gsos.h
  compile S.IO.C keep=o/S.IO

o/SCREEN.a: SCREEN.C stevie.h env.h ascii.h keymap.h param.h term.h macros.h gsos.h
  compile SCREEN.C keep=o/SCREEN

o/search.a: search.c stevie.h env.h ascii.h keymap.h param.h term.h macros.h gsos.h regexp.h
  compile search.c keep=o/search

o/VERSION.a: VERSION.C
  compile VERSION.C keep=o/VERSION

o/normal.a: normal.c stevie.h env.h ascii.h keymap.h param.h term.h macros.h  gsos.h
  compile normal.c keep=o/normal

vi: o/sio.a o/version.a o/search.a o/screen.a o/alloc.a o/cmdline.a \
    o/regsub.a o/regexp.a o/param.a o/normal.a o/mk.a o/gsos.a o/format.l.a \
    o/misccmds.a o/mark.a o/main.a o/linefunc.a o/inc.a o/help.a \
    o/fileio.a o/edit.a o/dec.a o/charset.a
  compile linkscr keep=vi
