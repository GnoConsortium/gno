* -------------------------------------------------------------------
*
* Example of routine to re-parse the commandline arguments to
* recognize both " and ' as quote characters.
*
* Written by Dave Tribby beginning November 1999
*
* Summary:  extern int reparse(char *argv[], char *commandline);
*
* Method: call the GNO routine ~GNO_PARSEARG.
*
* -------------------------------------------------------------------

dummy	START		; This segment ends up in .root
	END

	case	on
reparse	START

; NOTE: ~GNO_PARSEARG expects commandline parameter to be the "raw"
;	value passed by the shell, so it skips the first 8 characters.
;	Need to adjust pointer back 8 bytes.

	lda	$8,s	; Get commandline parameter, low-order word.
	sec
	sbc	#8	; Subtract 8.
	sta	$8,s
	lda	$a,s	; Adjust high-order word, if necessary.
	sbc	#0
	sta	$a,s

	jml	>~GNO_PARSEARG	; Let ~GNO_PARSEARG handle the rest.

	END
