ls.root: ls.c
   compile -i ls.c keep=ls

itqsort.a: itqsort.c
  compile +O itqsort.c keep=itqsort

ls: ls.root itqsort.a
   link ls itqsort keep=ls
