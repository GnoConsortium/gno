passwd.a: passwd.cc
  chtyp -l cc passwd.cc
  compile +w passwd.cc keep=passwd

passwd: passwd.a
  link passwd keep=passwd
  setvers passwd \'GNO passwd^by Eric Shepherd\' v1.0.0
