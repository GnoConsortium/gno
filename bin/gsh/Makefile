#
# GNO Shell Makefile
# by T Meekins
#

o/main.root: main.asm
  purge ; compile main.asm keep=o/main

o/shell.root: shell.asm m/shell.mac
  purge ; compile shell.asm keep=o/shell

o/history.root: history.asm m/history.mac
  purge ; compile history.asm keep=o/history

o/prompt.root: prompt.asm m/prompt.mac
  purge ; compile prompt.asm keep=o/prompt

o/cmd.root: cmd.asm m/cmd.mac
  purge ; compile cmd.asm keep=o/cmd

o/expand.root: expand.asm m/expand.mac
  purge ; compile expand.asm keep=o/expand

o/invoke.root: invoke.asm m/invoke.mac
  purge ; compile invoke.asm keep=o/invoke

o/shellutil.root: shellutil.asm m/shellutil.mac
  purge ; compile shellutil.asm keep=o/shellutil

o/builtin.root: builtin.asm m/builtin.mac
  purge ; compile builtin.asm keep=o/builtin

o/hash.root: hash.asm m/hash.mac
  purge ; compile hash.asm keep=o/hash

o/alias.root: alias.asm m/alias.mac
  purge ; compile alias.asm keep=o/alias

o/dir.root: dir.asm m/dir.mac
  purge ; compile dir.asm keep=o/dir

o/shellvar.root: shellvar.asm m/shellvar.mac
  purge ; compile shellvar.asm keep=o/shellvar

o/jobs.root: jobs.asm m/jobs.mac
  purge ; compile jobs.asm keep=o/jobs

o/sv.root: sv.asm m/sv.mac
  purge ; compile sv.asm keep=o/sv

o/stdio.root: stdio.asm m/stdio.mac
  purge ; compile stdio.asm keep=o/stdio

o/orca.root: orca.asm m/orca.mac
  purge ; compile orca.asm keep=o/orca

o/edit.root: edit.asm m/edit.mac
  purge ; compile edit.asm keep=o/edit

o/term.root: term.asm m/term.mac
  purge ; compile term.asm keep=o/term

o/bufpool.root: bufpool.asm m/bufpool.mac
  purge ; compile bufpool.asm keep=o/bufpool

shell: o/main.root \
       o/shell.root \
       o/history.root \
       o/prompt.root \
       o/cmd.root \
       o/expand.root \
       o/invoke.root \
       o/shellutil.root \
       o/builtin.root \
       o/hash.root \
       o/alias.root \
       o/shellvar.root \
       o/jobs.root \
       o/dir.root \
       o/sv.root \
       o/stdio.root \
       o/orca.root \
       o/edit.root \
       o/term.root \
       o/bufpool.root \
       direct.root
  pwd ; purge ; compile link.script keep=gsh
