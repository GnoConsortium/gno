main.a: main.c pathnames.h
    compile -i main.c keep=main

init.a: init.c pathnames.h
    compile -i init.c keep=init

subr.a: subr.c
    compile -i subr.c keep=subr

gettytab.a: gettytab.c pathnames.h
    compile -i +w gettytab.c keep=gettytab

ttydflts.a: ttydefaults.c
    compile -i ttydefaults.c keep=ttydflts

getty: main.c init.c subr.c gettytab.c ttydefaults.c
    compile linkfile
