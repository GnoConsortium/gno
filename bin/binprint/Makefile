binprint.a: binprint.c
    compile -p binprint.c keep=binprint

bprasm.a: binprint.asm
    compile -p binprint.asm

binprint: binprint.a bprasm.a
    link -p binprint bprasm keep=binprint
