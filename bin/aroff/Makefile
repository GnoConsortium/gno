#
# Makefile for aroff
#  AWGS WordProc -> Text formatter
#

awgs.root: awgs.c awgs.h
  compile awgs.c keep=awgs

print.root: print.c awgs.h
  compile print.c keep=print

awgs: awgs.root
  link awgs print 2/direct256 keep=aroff
