;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 2.9.2 #5492 (Aug 17 2009) (MINGW32)
; This file was generated Thu Sep 17 21:03:48 2009
;--------------------------------------------------------
; PIC port for the 14-bit core
;--------------------------------------------------------
;	.file	"cylon.c"
	list	p=16f648A
	radix dec
	include "p16f648A.inc"
;--------------------------------------------------------
; external declarations
;--------------------------------------------------------
	extern	_init_hw
	extern	_delay_us
	extern	_delay
	extern	_CCP1CON_bits
	extern	_CMCON_bits
	extern	_EECON1_bits
	extern	_INTCON_bits
	extern	_OPTION_REG_bits
	extern	_PCON_bits
	extern	_PIE1_bits
	extern	_PIR1_bits
	extern	_PORTA_bits
	extern	_PORTB_bits
	extern	_RCSTA_bits
	extern	_STATUS_bits
	extern	_T1CON_bits
	extern	_T2CON_bits
	extern	_TRISA_bits
	extern	_TRISB_bits
	extern	_TXSTA_bits
	extern	_VRCON_bits
	extern	_INDF
	extern	_TMR0
	extern	_PCL
	extern	_STATUS
	extern	_FSR
	extern	_PORTA
	extern	_PORTB
	extern	_PCLATH
	extern	_INTCON
	extern	_PIR1
	extern	_TMR1L
	extern	_TMR1H
	extern	_T1CON
	extern	_TMR2
	extern	_T2CON
	extern	_CCPR1L
	extern	_CCPR1H
	extern	_CCP1CON
	extern	_RCSTA
	extern	_TXREG
	extern	_RCREG
	extern	_CMCON
	extern	_OPTION_REG
	extern	_TRISA
	extern	_TRISB
	extern	_PIE1
	extern	_PCON
	extern	_PR2
	extern	_TXSTA
	extern	_SPBRG
	extern	_EEDATA
	extern	_EEADR
	extern	_EECON1
	extern	_EECON2
	extern	_VRCON

	extern PSAVE
	extern SSAVE
	extern WSAVE
	extern STK12
	extern STK11
	extern STK10
	extern STK09
	extern STK08
	extern STK07
	extern STK06
	extern STK05
	extern STK04
	extern STK03
	extern STK02
	extern STK01
	extern STK00
;--------------------------------------------------------
; global declarations
;--------------------------------------------------------
	global	_cylon

;--------------------------------------------------------
; global definitions
;--------------------------------------------------------
;--------------------------------------------------------
; absolute symbol definitions
;--------------------------------------------------------
;--------------------------------------------------------
; compiler-defined variables
;--------------------------------------------------------
UDL_cylon_0	udata
r0x1018	res	1
r0x1019	res	1
r0x101A	res	1
r0x101B	res	1
r0x101C	res	1
r0x101D	res	1
r0x101E	res	1
_cylon_cylon_bits_1_1	res	24
;--------------------------------------------------------
; initialized data
;--------------------------------------------------------
;--------------------------------------------------------
; overlayable items in internal ram 
;--------------------------------------------------------
;	udata_ovr
;--------------------------------------------------------
; code
;--------------------------------------------------------
code_cylon	code
;***
;  pBlock Stats: dbName = C
;***
;entry:  _cylon	;Function start
; 2 exit points
;has an exit
;functions called:
;   _delay
;   _delay
;   _delay
;   _delay
;   _delay
;   _delay
;   _delay
;   _delay
;7 compiler assigned registers:
;   r0x1018
;   r0x1019
;   r0x101A
;   r0x101B
;   r0x101C
;   r0x101D
;   r0x101E
;; Starting pCode block
_cylon	;Function start
; 2 exit points
;	.line	10; "cylon.c"	void cylon(unsigned char cylon_style) {
	BANKSEL	r0x1018
	MOVWF	r0x1018
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
;	.line	14; "cylon.c"	const unsigned char cylon_bits[] = {
	BANKSEL	_cylon_cylon_bits_1_1
	CLRF	(_cylon_cylon_bits_1_1 + 0)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_cylon_cylon_bits_1_1 + 1)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x02
	MOVWF	(_cylon_cylon_bits_1_1 + 2)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_cylon_cylon_bits_1_1 + 3)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x03
	MOVWF	(_cylon_cylon_bits_1_1 + 4)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_cylon_cylon_bits_1_1 + 5)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x01
	MOVWF	(_cylon_cylon_bits_1_1 + 6)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x80
	MOVWF	(_cylon_cylon_bits_1_1 + 7)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_cylon_cylon_bits_1_1 + 8)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0xc0
	MOVWF	(_cylon_cylon_bits_1_1 + 9)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_cylon_cylon_bits_1_1 + 10)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x70
	MOVWF	(_cylon_cylon_bits_1_1 + 11)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_cylon_cylon_bits_1_1 + 12)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x38
	MOVWF	(_cylon_cylon_bits_1_1 + 13)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x08
	MOVWF	(_cylon_cylon_bits_1_1 + 14)
	MOVWF	(_cylon_cylon_bits_1_1 + 15)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x0c
	MOVWF	(_cylon_cylon_bits_1_1 + 16)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_cylon_cylon_bits_1_1 + 17)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x04
	MOVWF	(_cylon_cylon_bits_1_1 + 18)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x01
	MOVWF	(_cylon_cylon_bits_1_1 + 19)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_cylon_cylon_bits_1_1 + 20)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x01
	MOVWF	(_cylon_cylon_bits_1_1 + 21)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_cylon_cylon_bits_1_1 + 22)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_cylon_cylon_bits_1_1 + 23)
;	.line	32; "cylon.c"	if(cylon_style == 0) {
	BANKSEL	r0x1018
	MOVF	r0x1018,W
	BTFSS	STATUS,2
	GOTO	_00121_DS_
;	.line	36; "cylon.c"	for(i = 0; i < sizeof(cylon_bits); i=i+2) 
	CLRF	r0x1019
;unsigned compare: left < lit(0x18=24), size=1
_00123_DS_
	MOVLW	0x18
	BANKSEL	r0x1019
	SUBWF	r0x1019,W
	BTFSC	STATUS,0
	GOTO	_00126_DS_
;genSkipc:3080: created from rifx:0x2260cc
;	.line	38; "cylon.c"	PORTA = cylon_bits[i];
	MOVF	r0x1019,W
	ADDLW	(_cylon_cylon_bits_1_1 + 0)
	MOVWF	r0x101A
	MOVLW	high (_cylon_cylon_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x101B
	MOVF	r0x101A,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x101B,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTA
	MOVWF	_PORTA
;	.line	39; "cylon.c"	PORTB = cylon_bits[i+1];
	BANKSEL	r0x1019
	INCF	r0x1019,W
	MOVWF	r0x101A
	ADDLW	(_cylon_cylon_bits_1_1 + 0)
	MOVWF	r0x101A
	MOVLW	high (_cylon_cylon_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x101C
	MOVF	r0x101A,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x101C,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTB
	MOVWF	_PORTB
;	.line	40; "cylon.c"	delay(CYLON_SCAN_DELAY);
	MOVLW	0x32
	PAGESEL	_delay
	CALL	_delay
	PAGESEL	$
;	.line	36; "cylon.c"	for(i = 0; i < sizeof(cylon_bits); i=i+2) 
	BANKSEL	r0x1019
	INCF	r0x1019,F
	INCF	r0x1019,F
	GOTO	_00123_DS_
_00126_DS_
;	.line	44; "cylon.c"	while (1)
	MOVLW	0x16
	BANKSEL	r0x1019
	MOVWF	r0x1019
_00108_DS_
;	.line	46; "cylon.c"	PORTA = cylon_bits[i];
	BANKSEL	r0x1019
	MOVF	r0x1019,W
	ADDLW	(_cylon_cylon_bits_1_1 + 0)
	MOVWF	r0x101A
	MOVLW	high (_cylon_cylon_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x101C
	MOVF	r0x101A,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x101C,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTA
	MOVWF	_PORTA
;	.line	47; "cylon.c"	PORTB = cylon_bits[i+1];
	BANKSEL	r0x1019
	INCF	r0x1019,W
	MOVWF	r0x101A
	ADDLW	(_cylon_cylon_bits_1_1 + 0)
	MOVWF	r0x101A
	MOVLW	high (_cylon_cylon_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x101D
	MOVF	r0x101A,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x101D,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTB
	MOVWF	_PORTB
;	.line	48; "cylon.c"	delay(CYLON_SCAN_DELAY);
	MOVLW	0x32
	PAGESEL	_delay
	CALL	_delay
	PAGESEL	$
;	.line	49; "cylon.c"	if (i==0) break;
	BANKSEL	r0x1019
	MOVF	r0x1019,W
	BTFSC	STATUS,2
	GOTO	_00131_DS_
;	.line	50; "cylon.c"	i=i-2;
	MOVLW	0xfe
	ADDWF	r0x1019,F
	GOTO	_00108_DS_
_00121_DS_
;	.line	53; "cylon.c"	} else if(cylon_style == 1) {
	BANKSEL	r0x1018
	MOVF	r0x1018,W
	XORLW	0x01
	BTFSS	STATUS,2
	GOTO	_00118_DS_
;	.line	57; "cylon.c"	for(i = 0; i < sizeof(cylon_bits); i+=2) 
	CLRF	r0x1019
;unsigned compare: left < lit(0x18=24), size=1
_00127_DS_
	MOVLW	0x18
	BANKSEL	r0x1019
	SUBWF	r0x1019,W
	BTFSC	STATUS,0
	GOTO	_00131_DS_
;genSkipc:3080: created from rifx:0x2260cc
;	.line	59; "cylon.c"	PORTA = cylon_bits[i];
	MOVF	r0x1019,W
	ADDLW	(_cylon_cylon_bits_1_1 + 0)
	MOVWF	r0x101A
	MOVLW	high (_cylon_cylon_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x101D
	MOVF	r0x101A,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x101D,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTA
	MOVWF	_PORTA
;	.line	60; "cylon.c"	PORTB = cylon_bits[i+1];
	BANKSEL	r0x1019
	INCF	r0x1019,W
	MOVWF	r0x101A
	ADDLW	(_cylon_cylon_bits_1_1 + 0)
	MOVWF	r0x101A
	MOVLW	high (_cylon_cylon_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x101E
	MOVF	r0x101A,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x101E,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTB
	MOVWF	_PORTB
;	.line	61; "cylon.c"	delay(CYLON_SCAN_DELAY);
	MOVLW	0x32
	PAGESEL	_delay
	CALL	_delay
	PAGESEL	$
;	.line	57; "cylon.c"	for(i = 0; i < sizeof(cylon_bits); i+=2) 
	BANKSEL	r0x1019
	INCF	r0x1019,F
	INCF	r0x1019,F
	GOTO	_00127_DS_
_00118_DS_
;	.line	65; "cylon.c"	} else if(cylon_style == 2) {
	BANKSEL	r0x1018
	MOVF	r0x1018,W
	XORLW	0x02
	BTFSS	STATUS,2
	GOTO	_00131_DS_
;	.line	70; "cylon.c"	while (1)
	MOVLW	0x16
	MOVWF	r0x1018
_00113_DS_
;	.line	72; "cylon.c"	PORTA = cylon_bits[i];
	BANKSEL	r0x1018
	MOVF	r0x1018,W
	ADDLW	(_cylon_cylon_bits_1_1 + 0)
	MOVWF	r0x1019
	MOVLW	high (_cylon_cylon_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x101A
	MOVF	r0x1019,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x101A,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTA
	MOVWF	_PORTA
;	.line	73; "cylon.c"	PORTB = cylon_bits[i+1];
	BANKSEL	r0x1018
	INCF	r0x1018,W
	MOVWF	r0x1019
	ADDLW	(_cylon_cylon_bits_1_1 + 0)
	MOVWF	r0x1019
	MOVLW	high (_cylon_cylon_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x101E
	MOVF	r0x1019,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x101E,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTB
	MOVWF	_PORTB
;	.line	74; "cylon.c"	delay(CYLON_SCAN_DELAY);
	MOVLW	0x32
	PAGESEL	_delay
	CALL	_delay
	PAGESEL	$
;	.line	75; "cylon.c"	if (i==0) break;
	BANKSEL	r0x1018
	MOVF	r0x1018,W
	BTFSC	STATUS,2
	GOTO	_00131_DS_
;	.line	76; "cylon.c"	i-=2;
	MOVLW	0xfe
	ADDWF	r0x1018,F
	GOTO	_00113_DS_
_00131_DS_
	RETURN	
; exit point of _cylon


;	code size estimation:
;	  205+   34 =   239 instructions (  546 byte)

	end
