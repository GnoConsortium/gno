sim.root: sim.asm sim.mac simequates.equ
  compile sim.asm keep=sim

sim: sim.root
  link sim keep=sim
  chtyp -t \$B6 sim

simlib.root: simlib.asm simlib.mac simequates.equ
  compile simlib.asm keep=simlib

libsim: simlib.root
  rm -f libsim
  makelib libsim +simlib.root +simlib.a

test: test.c libsim
  compile test.c keep=test
  link test libsim keep=test
