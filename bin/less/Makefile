o/gsos.a: gsos.c
  compile gsos.c keep=o/gsos

o/ttyin.a: ttyin.c less.h defines.h proto.h
  compile ttyin.c keep=o/ttyin

o/edit.a: edit.c less.h defines.h proto.h
  compile edit.c keep=o/edit

o/lsystem.a: lsystem.c less.h defines.h proto.h position.h
  compile lsystem.c keep=o/lsystem

o/line.a: line.c less.h defines.h proto.h
  compile line.c keep=o/line

o/help.a: help.c less.h defines.h proto.h
  compile help.c keep=o/help

o/output.a: output.c less.h defines.h proto.h
  compile output.c keep=o/output

o/position.a: position.c less.h defines.h proto.h position.h
  compile position.c keep=o/position

o/tags.a: tags.c less.h defines.h proto.h
  compile tags.c keep=o/tags

o/input.a: input.c less.h defines.h proto.h
  compile input.c keep=o/input

o/command.a: command.c less.h defines.h proto.h position.h option.h cmd.h
  compile command.c keep=o/command

o/optfunc.a: optfunc.c less.h defines.h proto.h option.h
  compile optfunc.c keep=o/optfunc

o/screen.a: screen.c less.h defines.h proto.h
  compile screen.c keep=o/screen

o/jump.a: jump.c less.h defines.h proto.h position.h
  compile jump.c keep=o/jump

o/ifile.a: ifile.c less.h defines.h proto.h
  compile ifile.c keep=o/ifile

o/charset.a: charset.c less.h defines.h proto.h
  compile charset.c keep=o/charset

o/search.a: search.c less.h defines.h proto.h position.h
  compile search.c keep=o/search

o/mark.a: mark.c less.h defines.h proto.h position.h
  compile mark.c keep=o/mark

o/os.a: os.c less.h defines.h proto.h
  compile os.c keep=o/os

o/main.a: main.c less.h defines.h proto.h position.h
  compile main.c keep=o/main

o/option.a: option.c less.h defines.h proto.h option.h
  compile option.c keep=o/option

o/opttbl.a: opttbl.c less.h defines.h proto.h option.h
  compile opttbl.c keep=o/opttbl

o/forwback.a: forwback.c less.h defines.h proto.h position.h
  compile forwback.c keep=o/forwback

o/version.a: version.c less.h defines.h proto.h
  compile version.c keep=o/version

o/signal.a: signal.c less.h defines.h proto.h
  compile signal.c keep=o/signal

o/cmdbuf.a: cmdbuf.c less.h defines.h proto.h
  compile cmdbuf.c keep=o/cmdbuf

o/linenum.a: linenum.c less.h defines.h proto.h position.h
  compile linenum.c keep=o/linenum

o/ch.a: ch.c less.h defines.h proto.h
  compile ch.c keep=o/ch

o/decode.a: decode.c less.h defines.h proto.h cmd.h
  compile decode.c keep=o/decode

o/filename.a: filename.c less.h defines.h proto.h
  compile filename.c keep=o/filename

o/prompt.a: prompt.c less.h defines.h proto.h position.h
  compile prompt.c keep=o/prompt

o/brac.a: brac.c less.h defines.h proto.h position.h
  compile brac.c keep=o/brac

less: o/BRAC.A o/FILENAME.A o/LINENUM.A o/OS.A o/TAGS.A o/CH.A o/FORWBACK.A \
      o/LSYSTEM.A o/OUTPUT.A o/TTYIN.A o/CHARSET.A o/HELP.A o/MAIN.A \
      o/POSITION.A o/VERSION.A o/CMDBUF.A o/IFILE.A o/MARK.A o/PROMPT.A \
      o/COMMAND.A o/INPUT.A o/OPTFUNC.A o/SCREEN.A o/DECODE.A o/JUMP.A \
      o/OPTION.A o/SEARCH.A o/EDIT.A o/LINE.A o/OPTTBL.A o/SIGNAL.A o/GSOS.A
 purge
 compile linkscr
 purge
