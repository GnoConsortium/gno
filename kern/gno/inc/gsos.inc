*	$Id: gsos.inc,v 1.1 1998/02/02 08:20:54 taubert Exp $
* if you change FDsize, look for the comment "a * FDsize". You must
* change code there.

FDsize	gequ  16
FDrefNum	gequ  0                  ; type and level must be so
FDrefType	gequ  2
FDrefLevel	gequ  4
FDrefFlags	gequ  6
FDNLenableMask	gequ  8
FDNLnumChars	gequ  10
FDNLtable	gequ  12

FDgsos	gequ  0                  ; refTypes
FDpipe	gequ  1
FDtty	gequ  2
FDsocket	gequ  3

FDTCount	gequ  0
FDTLevel	gequ  2
FDTLevelMode	gequ  4
FDTfdTableSize	gequ  6
FDTTable	gequ  8
