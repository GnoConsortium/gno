*	$Id: err.asm,v 1.1 1998/02/02 08:19:20 taubert Exp $
**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*   Derek Taubert
*
**************************************************************************
*
* ERR.ASM
*   By Tim Meekins
*
* Displays error messages for
*   GS/OS
*   Loader
*   Memory Manager
*   Resource manager
*   GNO ($FFxx)
*
**************************************************************************

	mcopy m/err.mac
	case	on

printError	START KERN2

	proc

	sta   err
	cmp   #0
	bne   gooderr
	procendL

gooderr	anop
	Int2Hex (err,#str1+1,#4)
	ErrWriteCString #str1

	ldx   #0
findtoolloop	lda   tooltbl+2,x
	bmi   foundtool
	lda   err
	and   #$FF00
	cmp   tooltbl,x
	beq   foundtool
	add2  @x,#8,@x
	bra   findtoolloop

foundtool	lda   tooltbl+6,x
	tay
	lda   tooltbl+4,x
	ErrWriteString @ya

	ldx   #0
finderrloop	lda   errtbl,x
	cmp   #$FFFF
	beq   founderr
	cmp   err
	beq   founderr
	add2  @x,#6,@x
	bra   finderrloop

founderr	lda   errtbl+4,x
	tay
	lda   errtbl+2,x
	ErrWriteLine @ya

	procendL

err	ds    2
str1	dc    c'$0000 ',h'00'

tooltbl	dc    i4'$0000',a4'tstr1'
	dc    i4'$0200',a4'tstr3'
	dc    i4'$1100',a4'tstr2'
	dc    i4'$1E00',a4'tstr4'
	dc    i4'$FF00',a4'tstr5'
	dc    i4'-1',a4'tstr0'

errtbl	dc    i'$0001',a4'err0001'
	dc    i'$0004',a4'err0004'
	dc	i'$0005',a4'err0005'
	dc    i'$0006',a4'err0006'
	dc    i'$0007',a4'err0007'
	dc    i'$0010',a4'err0010'
	dc    i'$0011',a4'err0011'
	dc    i'$001F',a4'err001F'
	dc    i'$0020',a4'err0020'
	dc    i'$0021',a4'err0021'
	dc    i'$0022',a4'err0022'
	dc    i'$0023',a4'err0023'
	dc    i'$0024',a4'err0024'
	dc    i'$0025',a4'err0025'
	dc    i'$0026',a4'err0026'
	dc    i'$0027',a4'err0027'
	dc    i'$0028',a4'err0028'
	dc    i'$0029',a4'err0029'
	dc    i'$002B',a4'err002B'
	dc    i'$002C',a4'err002C'
	dc    i'$002D',a4'err002D'
	dc    i'$002E',a4'err002E'
	dc    i'$002F',a4'err002F'
	dc    i'$0040',a4'err0040'
	dc	i'$0042',a4'err0042'
	dc    i'$0043',a4'err0043'
	dc    i'$0044',a4'err0044'
	dc    i'$0045',a4'err0045'
	dc    i'$0046',a4'err0046'
	dc    i'$0047',a4'err0047'
	dc    i'$0048',a4'err0048'
	dc    i'$0049',a4'err0049'
	dc    i'$004A',a4'err004A'
	dc    i'$004B',a4'err004B'
	dc    i'$004C',a4'err004C'
	dc    i'$004D',a4'err004D'
	dc    i'$004E',a4'err004E'
	dc    i'$004F',a4'err004F'
	dc    i'$0050',a4'err0050'
	dc    i'$0051',a4'err0051'
	dc    i'$0052',a4'err0052'
	dc    i'$0053',a4'err0053'
	dc    i'$0054',a4'err0054'
	dc	i'$0055',a4'err0055'
	dc	i'$0056',a4'err0056'
	dc    i'$0057',a4'err0057'
	dc    i'$0058',a4'err0058'
	dc    i'$0059',a4'err0059'
	dc    i'$005A',a4'err005A'
	dc    i'$005B',a4'err005B'
	dc    i'$005C',a4'err005C'
	dc    i'$005D',a4'err005D'
	dc	i'$005E',a4'err005E'
	dc    i'$005F',a4'err005F'
	dc    i'$0060',a4'err0060'
	dc    i'$0061',a4'err0061'
	dc    i'$0062',a4'err0062'
	dc    i'$0063',a4'err0063'
	dc    i'$0064',a4'err0064'
	dc	i'$0065',a4'err0065'
	dc    i'$0067',a4'err0067'
	dc    i'$0069',a4'err0069'
	dc    i'$0070',a4'err0070'
	dc    i'$0071',a4'err0071'
	dc    i'$0201',a4'err0201'
	dc    i'$0202',a4'err0202'
	dc    i'$0203',a4'err0203'
	dc    i'$0204',a4'err0204'
	dc    i'$0205',a4'err0205'
	dc    i'$0206',a4'err0206'
	dc    i'$0207',a4'err0207'
	dc    i'$0208',a4'err0208'
	dc    i'$1101',a4'err1101'
	dc    i'$1102',a4'err1102'
	dc    i'$1103',a4'err1103'
	dc    i'$1104',a4'err1104'
	dc	i'$1105',a4'err1105'
	dc    i'$1107',a4'err1107'
	dc    i'$1108',a4'err1108'
	dc    i'$1109',a4'err1109'
	dc    i'$110A',a4'err110A'
	dc    i'$110B',a4'err110B'
	dc    i'$1E01',a4'err1E01'
	dc    i'$1E02',a4'err1E02'
	dc    i'$1E03',a4'err1E03'
	dc    i'$1E04',a4'err1E04'
	dc    i'$1E05',a4'err1E05'
	dc    i'$1E06',a4'err1E06'
	dc    i'$1E07',a4'err1E07'
	dc    i'$1E08',a4'err1E08'
	dc    i'$1E09',a4'err1E09'
	dc    i'$1E0A',a4'err1E0A'
	dc    i'$1E0B',a4'err1E0B'
	dc    i'$1E0C',a4'err1E0C'
	dc    i'$1E0D',a4'err1E0D'
	dc    i'$1E0E',a4'err1E0E'
	dc	i'$FF00',a4'errFF00'
	dc	i'$FF02',a4'errFF02'
	dc	i'$FF03',a4'errFF03'
	dc	i'$FF04',a4'errFF04'
	dc	i'$FF05',a4'errFF05'
	dc	i'$FF06',a4'errFF06'
	dc    i'$FFFF',a4'errFFFF'

tstr0	str   'Unknown '
tstr1	str   'GS/OS: '
tstr2	str   'Loader: '
tstr3	str   'Memory Manager: '
tstr4	str   'Resource: '
tstr5	str   'GNO: '

err0001	str   'Bad GS/OS call number'
err0004	str   'Parameter count out of range'
err0005	str	'Parameter pointer out of range'
err0006	str   'Communication error in IWM'
err0007	str   'GS/OS is busy'
err0010	str   'Device wasn''t found'
err0011	str   'Invalid device number requested'
err001F	str   'Interrupt devices are not supported'
err0020	str   'Invalid request'
err0021	str   'Invalid control or status code'
err0022	str   'Bad call parameter'
err0023	str   'Character device not open'
err0024	str   'Character device already open'
err0025	str   'Interrupt table full'
err0026	str   'Resources not available'
err0027	str   'Disk I/O error'
err0028	str   'No device connected'
err0029	str   'Driver is busy'
err002B	str   'Device is write-protected'
err002C	str   'Invalid byte count'
err002D	str   'Invalid block address'
err002E	str   'Disk has been switched'
err002F	str   'Device off line or no media present'
err0040	str   'Invalid pathname syntax'
err0042	str	'Too many files open'
err0043	str   'Invalid reference number'
err0044	str   'Subdirectory does not exist'
err0045	str   'Volume not found'
err0046	str   'File not found'
err0047	str   'Create or rename with existing name'
err0048	str   'Volume full'
err0049	str   'Volume directory full'
err004A	str   'Version error (Incompatible file format)'
err004B	str   'Unsupported storage type'
err004C	str   'End of file encountered'
err004D	str   'Position out of range'
err004E	str   'Access not allowed'
err004F	str   'Buffer too small'
err0050	str   'File is already open'
err0051	str   'Directory error'
err0052	str   'Unknown volume type'
err0053	str   'Parameter out of range'
err0054	str   'Out of memory'
err0055	str	'Block table full'
err0056	str	'Invalid buffer'
err0057	str   'Duplicate volume name'
err0058	str   'Not a block device'
err0059	str   'Level specified outside of legal range'
err005A	str   'Block number too large'
err005B	str   'Invalid path names for ChangePath'
err005C	str   'Not an executable'
err005D	str   'Operating system not supported'
err005E	str	'/RAM cannot be removed'
err005F	str   'Too many applications on stack'
err0060	str   'Data unavailable'
err0061	str   'End of directory has been reached'
err0062	str   'Invalid FST call class'
err0063	str   'File does not contain required resource'
err0064	str   'FST ID is invalid'
err0065	str	'Invalid FST operation'
err0067	str   'Device exists with same name as replacement name'
err0069	str   'I/O terminated due to NewLine'
err0070	str   'Cannot expand device, resource fork already exists'
err0071	str   'Cannot add resource fork to this type of file'
err0201	str   'Unable to allocate block'
err0202	str   'Illegal operation on an empty handle'
err0203	str   'Empty handle expected for this operation'
err0204	str   'Illegal operation on a locked or immovable block'
err0205	str   'Attempt to purge an unpurgable block'
err0206	str   'Invalid handle'
err0207	str   'Invalid user ID'
err0208	str   'Illegal operation with specified attributes'
err1101	str   'Entry not found'
err1102	str   'OMF version error'
err1103	str   'Pathname error'
err1104	str   'File not in load file'
err1105	str	'Loader is busy'
err1107	str   'File version error'
err1108	str   'User ID error'
err1109	str   'Segment number out of sequence'
err110A	str   'Illegal load record found'
err110B	str   'Load segment is foreign'
err1E01	str   'Resource fork not empty'
err1E02	str   'Resource fork not correctly formatted'
err1E03	str   'No converter routine for resource type'
err1E04	str   'No current resource file'
err1E05	str   'Specified resource ID is already in use'
err1E06	str   'Specified resource not found'
err1E07	str   'Specified ID does not match an open file'
err1E08	str   'User ID not found'
err1E09	str   'No more resource IDs available'
err1E0A	str   'Index is out of range'
err1E0B	str   'System file is already open'
err1E0C	str   'Resource has been changed'
err1E0D	str   'Another converted already logged in'
err1E0E	str   'Volume full'
errFF00	str	'Too many arguments'
errFF02	str	'Extra < encountered'
errFF03	str	'Illegal < syntax'
errFF04	str	'Extra > encountered'
errFF05	str	'Illegal > syntax'
errFF06	str	'> and | conflict'

errFFFF	str   'error'

	END
