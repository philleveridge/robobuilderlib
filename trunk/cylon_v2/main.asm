;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 2.9.2 #5492 (Aug 17 2009) (MINGW32)
; This file was generated Fri Sep 18 00:00:14 2009
;--------------------------------------------------------
; PIC port for the 14-bit core
;--------------------------------------------------------
;	.file	"main.c"
	list	p=16f648A
	radix dec
	include "p16f648A.inc"
;--------------------------------------------------------
; config word 
;--------------------------------------------------------
	__config 0x3f38
;--------------------------------------------------------
; external declarations
;--------------------------------------------------------
	extern	_init_hw
	extern	_delay_us
	extern	_delay
	extern	_play_tone
	extern	_cylon
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
	extern	__sdcc_gsinit_startup
;--------------------------------------------------------
; global declarations
;--------------------------------------------------------
	global	_get_byte
	global	_put_byte
	global	_pattern
	global	_tune
	global	_main

	global PSAVE
	global SSAVE
	global WSAVE
	global STK12
	global STK11
	global STK10
	global STK09
	global STK08
	global STK07
	global STK06
	global STK05
	global STK04
	global STK03
	global STK02
	global STK01
	global STK00

sharebank udata_ovr 0x0070
PSAVE	res 1
SSAVE	res 1
WSAVE	res 1
STK12	res 1
STK11	res 1
STK10	res 1
STK09	res 1
STK08	res 1
STK07	res 1
STK06	res 1
STK05	res 1
STK04	res 1
STK03	res 1
STK02	res 1
STK01	res 1
STK00	res 1

;--------------------------------------------------------
; global definitions
;--------------------------------------------------------
;--------------------------------------------------------
; absolute symbol definitions
;--------------------------------------------------------
;--------------------------------------------------------
; compiler-defined variables
;--------------------------------------------------------
UDL_main_0	udata
r0x1033	res	1
r0x1034	res	1
r0x1035	res	1
r0x1036	res	1
r0x1037	res	1
r0x1038	res	1
r0x1039	res	1
r0x103B	res	1
r0x103A	res	1
_pattern_bits_1_1	res	48
_main_d_1_1	res	2
_main_i_1_1	res	1
;--------------------------------------------------------
; initialized data
;--------------------------------------------------------
;--------------------------------------------------------
; overlayable items in internal ram 
;--------------------------------------------------------
;	udata_ovr
;--------------------------------------------------------
; reset vector 
;--------------------------------------------------------
STARTUP	code 0x0000
	nop
	pagesel __sdcc_gsinit_startup
	goto	__sdcc_gsinit_startup
;--------------------------------------------------------
; code
;--------------------------------------------------------
code_main	code
;***
;  pBlock Stats: dbName = M
;***
;entry:  _main	;Function start
; 2 exit points
;has an exit
;functions called:
;   _init_hw
;   _pattern
;   _tune
;   _get_byte
;   _get_byte
;   _cylon
;   _cylon
;   _cylon
;   _tune
;   _get_byte
;   _get_byte
;   _pattern
;   _get_byte
;   _get_byte
;   _play_tone
;   _put_byte
;   _init_hw
;   _pattern
;   _tune
;   _get_byte
;   _get_byte
;   _cylon
;   _cylon
;   _cylon
;   _tune
;   _get_byte
;   _get_byte
;   _pattern
;   _get_byte
;   _get_byte
;   _play_tone
;   _put_byte
;5 compiler assigned registers:
;   STK00
;   r0x103A
;   r0x103B
;   STK02
;   STK01
;; Starting pCode block
_main	;Function start
; 2 exit points
;	.line	112; "main.c"	init_hw();
	PAGESEL	_init_hw
	CALL	_init_hw
	PAGESEL	$
;	.line	114; "main.c"	pattern();
	CALL	_pattern
;	.line	115; "main.c"	PORTA=(unsigned char)5; PORTB=(unsigned char)72;	// 01010 01010	Eyes
	MOVLW	0x05
	BANKSEL	_PORTA
	MOVWF	_PORTA
	MOVLW	0x48
	MOVWF	_PORTB
;	.line	116; "main.c"	tune();
	CALL	_tune
_00163_DS_
;	.line	120; "main.c"	d=get_byte();
	CALL	_get_byte
	BANKSEL	_main_d_1_1
	MOVWF	(_main_d_1_1 + 1)
	MOVF	STK00,W
;	.line	122; "main.c"	if (d == 'S') //start byte
	MOVWF	_main_d_1_1
	XORLW	0x53
	BTFSS	STATUS,2
	GOTO	_00163_DS_
	MOVF	(_main_d_1_1 + 1),W
	XORLW	0x00
	BTFSS	STATUS,2
	GOTO	_00163_DS_
;	.line	124; "main.c"	d=get_byte();
	CALL	_get_byte
	BANKSEL	_main_d_1_1
	MOVWF	(_main_d_1_1 + 1)
	MOVF	STK00,W
;	.line	129; "main.c"	switch (d)
	MOVWF	_main_d_1_1
	XORLW	0x41
	BTFSS	STATUS,2
	GOTO	_00181_DS_
	MOVF	(_main_d_1_1 + 1),W
	XORLW	0x00
	BTFSC	STATUS,2
	GOTO	_00147_DS_
_00181_DS_
	BANKSEL	_main_d_1_1
	MOVF	_main_d_1_1,W
	XORLW	0x42
	BTFSS	STATUS,2
	GOTO	_00182_DS_
	MOVF	(_main_d_1_1 + 1),W
	XORLW	0x00
	BTFSC	STATUS,2
	GOTO	_00148_DS_
_00182_DS_
	BANKSEL	_main_d_1_1
	MOVF	_main_d_1_1,W
	XORLW	0x43
	BTFSS	STATUS,2
	GOTO	_00183_DS_
	MOVF	(_main_d_1_1 + 1),W
	XORLW	0x00
	BTFSC	STATUS,2
	GOTO	_00149_DS_
_00183_DS_
	BANKSEL	_main_d_1_1
	MOVF	_main_d_1_1,W
	XORLW	0x4c
	BTFSS	STATUS,2
	GOTO	_00184_DS_
	MOVF	(_main_d_1_1 + 1),W
	XORLW	0x00
	BTFSC	STATUS,2
	GOTO	_00152_DS_
_00184_DS_
	BANKSEL	_main_d_1_1
	MOVF	_main_d_1_1,W
	XORLW	0x50
	BTFSS	STATUS,2
	GOTO	_00185_DS_
	MOVF	(_main_d_1_1 + 1),W
	XORLW	0x00
	BTFSC	STATUS,2
	GOTO	_00155_DS_
_00185_DS_
	BANKSEL	_main_d_1_1
	MOVF	_main_d_1_1,W
	XORLW	0x52
	BTFSS	STATUS,2
	GOTO	_00186_DS_
	MOVF	(_main_d_1_1 + 1),W
	XORLW	0x00
	BTFSC	STATUS,2
	GOTO	_00153_DS_
_00186_DS_
	BANKSEL	_main_d_1_1
	MOVF	_main_d_1_1,W
	XORLW	0x53
	BTFSS	STATUS,2
	GOTO	_00187_DS_
	MOVF	(_main_d_1_1 + 1),W
	XORLW	0x00
	BTFSC	STATUS,2
	GOTO	_00151_DS_
_00187_DS_
	BANKSEL	_main_d_1_1
	MOVF	_main_d_1_1,W
	XORLW	0x54
	BTFSS	STATUS,2
	GOTO	_00188_DS_
	MOVF	(_main_d_1_1 + 1),W
	XORLW	0x00
	BTFSC	STATUS,2
	GOTO	_00150_DS_
_00188_DS_
	BANKSEL	_main_d_1_1
	MOVF	_main_d_1_1,W
	XORLW	0x58
	BTFSS	STATUS,2
	GOTO	_00189_DS_
	MOVF	(_main_d_1_1 + 1),W
	XORLW	0x00
	BTFSC	STATUS,2
	GOTO	_00154_DS_
_00189_DS_
	BANKSEL	_main_d_1_1
	MOVF	_main_d_1_1,W
	XORLW	0x70
	BTFSS	STATUS,2
	GOTO	_00190_DS_
	MOVF	(_main_d_1_1 + 1),W
	XORLW	0x00
	BTFSC	STATUS,2
	GOTO	_00156_DS_
_00190_DS_
	BANKSEL	_main_d_1_1
	MOVF	_main_d_1_1,W
	XORLW	0x74
	BTFSS	STATUS,2
	GOTO	_00191_DS_
	MOVF	(_main_d_1_1 + 1),W
	XORLW	0x00
	BTFSC	STATUS,2
	GOTO	_00157_DS_
_00191_DS_
	GOTO	_00158_DS_
_00147_DS_
;	.line	132; "main.c"	cylon(0);
	MOVLW	0x00
	PAGESEL	_cylon
	CALL	_cylon
	PAGESEL	$
;	.line	133; "main.c"	break;
	GOTO	_00159_DS_
_00148_DS_
;	.line	135; "main.c"	cylon(1);
	MOVLW	0x01
	PAGESEL	_cylon
	CALL	_cylon
	PAGESEL	$
;	.line	136; "main.c"	break;
	GOTO	_00159_DS_
_00149_DS_
;	.line	138; "main.c"	cylon(2);
	MOVLW	0x02
	PAGESEL	_cylon
	CALL	_cylon
	PAGESEL	$
;	.line	139; "main.c"	break;
	GOTO	_00159_DS_
_00150_DS_
;	.line	141; "main.c"	tune();
	CALL	_tune
;	.line	142; "main.c"	break;
	GOTO	_00159_DS_
_00151_DS_
;	.line	144; "main.c"	PORTA=(unsigned char)11; PORTB=(unsigned char)152;
	MOVLW	0x0b
	BANKSEL	_PORTA
	MOVWF	_PORTA
	MOVLW	0x98
	MOVWF	_PORTB
;	.line	145; "main.c"	break;
	GOTO	_00159_DS_
_00152_DS_
;	.line	147; "main.c"	PORTA=(unsigned char)12; PORTB=(unsigned char)225;
	MOVLW	0x0c
	BANKSEL	_PORTA
	MOVWF	_PORTA
	MOVLW	0xe1
	MOVWF	_PORTB
;	.line	148; "main.c"	break;
	GOTO	_00159_DS_
_00153_DS_
;	.line	150; "main.c"	PORTA=(unsigned char)13; PORTB=(unsigned char)200;
	MOVLW	0x0d
	BANKSEL	_PORTA
	MOVWF	_PORTA
	MOVLW	0xc8
	MOVWF	_PORTB
;	.line	151; "main.c"	break;
	GOTO	_00159_DS_
_00154_DS_
;	.line	153; "main.c"	PORTA=(unsigned char)5; PORTB=(unsigned char)72;
	MOVLW	0x05
	BANKSEL	_PORTA
	MOVWF	_PORTA
	MOVLW	0x48
	MOVWF	_PORTB
;	.line	154; "main.c"	break;
	GOTO	_00159_DS_
_00155_DS_
;	.line	156; "main.c"	i=get_byte();
	CALL	_get_byte
	BANKSEL	r0x103A
	MOVWF	r0x103A
	MOVF	STK00,W
	MOVWF	r0x103B
;	.line	157; "main.c"	PORTA=i;
	BANKSEL	_main_i_1_1
	MOVWF	_main_i_1_1
	BANKSEL	_PORTA
	MOVWF	_PORTA
;	.line	158; "main.c"	i=get_byte();
	CALL	_get_byte
	BANKSEL	r0x103A
	MOVWF	r0x103A
	MOVF	STK00,W
	MOVWF	r0x103B
;	.line	159; "main.c"	PORTB=i;
	BANKSEL	_main_i_1_1
	MOVWF	_main_i_1_1
	BANKSEL	_PORTB
	MOVWF	_PORTB
;	.line	160; "main.c"	break;
	GOTO	_00159_DS_
_00156_DS_
;	.line	162; "main.c"	pattern();
	CALL	_pattern
;	.line	163; "main.c"	break;
	GOTO	_00159_DS_
_00157_DS_
;	.line	165; "main.c"	d=get_byte();
	CALL	_get_byte
	BANKSEL	_main_d_1_1
	MOVWF	(_main_d_1_1 + 1)
	MOVF	STK00,W
	MOVWF	_main_d_1_1
;	.line	166; "main.c"	i=get_byte();
	CALL	_get_byte
	BANKSEL	r0x103A
	MOVWF	r0x103A
	MOVF	STK00,W
	MOVWF	r0x103B
;	.line	167; "main.c"	play_tone(d,i);
	BANKSEL	_main_i_1_1
	MOVWF	_main_i_1_1
	BANKSEL	r0x103B
	MOVWF	r0x103B
	CLRF	r0x103A
	MOVF	r0x103B,W
	MOVWF	STK02
	MOVLW	0x00
	MOVWF	STK01
	BANKSEL	_main_d_1_1
	MOVF	_main_d_1_1,W
	MOVWF	STK00
	MOVF	(_main_d_1_1 + 1),W
	PAGESEL	_play_tone
	CALL	_play_tone
	PAGESEL	$
;	.line	168; "main.c"	d='t';
	MOVLW	0x74
	BANKSEL	_main_d_1_1
	MOVWF	_main_d_1_1
	CLRF	(_main_d_1_1 + 1)
;	.line	169; "main.c"	break;
	GOTO	_00159_DS_
_00158_DS_
;	.line	171; "main.c"	d='z'; 		//error
	MOVLW	0x7a
	BANKSEL	_main_d_1_1
	MOVWF	_main_d_1_1
	CLRF	(_main_d_1_1 + 1)
_00159_DS_
;	.line	174; "main.c"	if (d) put_byte(d); //response
	BANKSEL	_main_d_1_1
	MOVF	_main_d_1_1,W
	IORWF	(_main_d_1_1 + 1),W
	BTFSC	STATUS,2
	GOTO	_00163_DS_
	MOVF	_main_d_1_1,W
	MOVWF	STK00
	MOVF	(_main_d_1_1 + 1),W
	CALL	_put_byte
	GOTO	_00163_DS_
	RETURN	
; exit point of _main

;***
;  pBlock Stats: dbName = C
;***
;entry:  _tune	;Function start
; 2 exit points
;has an exit
;functions called:
;   _play_tone
;   _play_tone
;   _play_tone
;   _play_tone
;   _play_tone
;   _play_tone
;   _play_tone
;   _play_tone
;   _play_tone
;   _play_tone
;3 compiler assigned registers:
;   STK02
;   STK01
;   STK00
;; Starting pCode block
_tune	;Function start
; 2 exit points
;	.line	100; "main.c"	play_tone(D5, DUR);
	MOVLW	0x28
	MOVWF	STK02
	MOVLW	0x00
	MOVWF	STK01
	MOVLW	0x53
	MOVWF	STK00
	MOVLW	0x03
	PAGESEL	_play_tone
	CALL	_play_tone
	PAGESEL	$
;	.line	101; "main.c"	play_tone(E5, DUR);
	MOVLW	0x28
	MOVWF	STK02
	MOVLW	0x00
	MOVWF	STK01
	MOVLW	0xf6
	MOVWF	STK00
	MOVLW	0x02
	PAGESEL	_play_tone
	CALL	_play_tone
	PAGESEL	$
;	.line	102; "main.c"	play_tone(D5, DUR);
	MOVLW	0x28
	MOVWF	STK02
	MOVLW	0x00
	MOVWF	STK01
	MOVLW	0x53
	MOVWF	STK00
	MOVLW	0x03
	PAGESEL	_play_tone
	CALL	_play_tone
	PAGESEL	$
;	.line	103; "main.c"	play_tone(G5, DUR);
	MOVLW	0x28
	MOVWF	STK02
	MOVLW	0x00
	MOVWF	STK01
	MOVLW	0x7d
	MOVWF	STK00
	MOVLW	0x02
	PAGESEL	_play_tone
	CALL	_play_tone
	PAGESEL	$
;	.line	104; "main.c"	play_tone(Fsh5, 2*DUR);
	MOVLW	0x50
	MOVWF	STK02
	MOVLW	0x00
	MOVWF	STK01
	MOVLW	0xa3
	MOVWF	STK00
	MOVLW	0x02
	PAGESEL	_play_tone
	CALL	_play_tone
	PAGESEL	$
	RETURN	
; exit point of _tune

;***
;  pBlock Stats: dbName = C
;***
;entry:  _pattern	;Function start
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
;   r0x1033
;   r0x1034
;   r0x1035
;   r0x1036
;   r0x1037
;   r0x1038
;   r0x1039
;; Starting pCode block
_pattern	;Function start
; 2 exit points
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
;	.line	40; "main.c"	const unsigned char bits[] = {
	BANKSEL	_pattern_bits_1_1
	CLRF	(_pattern_bits_1_1 + 0)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x01
	MOVWF	(_pattern_bits_1_1 + 1)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x04
	MOVWF	(_pattern_bits_1_1 + 2)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 3)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x08
	MOVWF	(_pattern_bits_1_1 + 4)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 5)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 6)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x08
	MOVWF	(_pattern_bits_1_1 + 7)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 8)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x10
	MOVWF	(_pattern_bits_1_1 + 9)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 10)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x20
	MOVWF	(_pattern_bits_1_1 + 11)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 12)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x40
	MOVWF	(_pattern_bits_1_1 + 13)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 14)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x80
	MOVWF	(_pattern_bits_1_1 + 15)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x01
	MOVWF	(_pattern_bits_1_1 + 16)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 17)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x02
	MOVWF	(_pattern_bits_1_1 + 18)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 19)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x04
	MOVWF	(_pattern_bits_1_1 + 20)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x01
	MOVWF	(_pattern_bits_1_1 + 21)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x08
	MOVWF	(_pattern_bits_1_1 + 22)
	MOVWF	(_pattern_bits_1_1 + 23)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 24)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x30
	MOVWF	(_pattern_bits_1_1 + 25)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 26)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0xc0
	MOVWF	(_pattern_bits_1_1 + 27)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x03
	MOVWF	(_pattern_bits_1_1 + 28)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 29)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x0c
	MOVWF	(_pattern_bits_1_1 + 30)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x01
	MOVWF	(_pattern_bits_1_1 + 31)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 32)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x38
	MOVWF	(_pattern_bits_1_1 + 33)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x01
	MOVWF	(_pattern_bits_1_1 + 34)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0xc0
	MOVWF	(_pattern_bits_1_1 + 35)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x02
	MOVWF	(_pattern_bits_1_1 + 36)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	CLRF	(_pattern_bits_1_1 + 37)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x03
	MOVWF	(_pattern_bits_1_1 + 38)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0xe0
	MOVWF	(_pattern_bits_1_1 + 39)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x0c
	MOVWF	(_pattern_bits_1_1 + 40)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x19
	MOVWF	(_pattern_bits_1_1 + 41)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x03
	MOVWF	(_pattern_bits_1_1 + 42)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0xe0
	MOVWF	(_pattern_bits_1_1 + 43)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x07
	MOVWF	(_pattern_bits_1_1 + 44)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x31
	MOVWF	(_pattern_bits_1_1 + 45)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0x08
	MOVWF	(_pattern_bits_1_1 + 46)
;/home/sdcc-builder/build/sdcc-build/orig/sdcc/src/pic/gen.c:5904: size=0/1, offset=0, AOP_TYPE(res)=8
	MOVLW	0xc8
	MOVWF	(_pattern_bits_1_1 + 47)
;	.line	69; "main.c"	for(i = 0; i < 20; i=i+2) 
	BANKSEL	r0x1033
	CLRF	r0x1033
;unsigned compare: left < lit(0x14=20), size=1
_00119_DS_
	MOVLW	0x14
	BANKSEL	r0x1033
	SUBWF	r0x1033,W
	BTFSC	STATUS,0
	GOTO	_00122_DS_
;genSkipc:3080: created from rifx:0x2260cc
;	.line	71; "main.c"	PORTA = bits[i];
	MOVF	r0x1033,W
	ADDLW	(_pattern_bits_1_1 + 0)
	MOVWF	r0x1034
	MOVLW	high (_pattern_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x1035
	MOVF	r0x1034,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x1035,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTA
	MOVWF	_PORTA
;	.line	72; "main.c"	PORTB = bits[i+1];
	BANKSEL	r0x1033
	INCF	r0x1033,W
	MOVWF	r0x1034
	ADDLW	(_pattern_bits_1_1 + 0)
	MOVWF	r0x1034
	MOVLW	high (_pattern_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x1036
	MOVF	r0x1034,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x1036,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTB
	MOVWF	_PORTB
;	.line	73; "main.c"	delay(500);
	MOVLW	0xf4
	PAGESEL	_delay
	CALL	_delay
	PAGESEL	$
;	.line	69; "main.c"	for(i = 0; i < 20; i=i+2) 
	BANKSEL	r0x1033
	INCF	r0x1033,F
	INCF	r0x1033,F
	GOTO	_00119_DS_
_00122_DS_
;	.line	76; "main.c"	for(i = 20; i < 30; i=i+2) 
	MOVLW	0x14
	BANKSEL	r0x1033
	MOVWF	r0x1033
;unsigned compare: left < lit(0x1E=30), size=1
_00123_DS_
	MOVLW	0x1e
	BANKSEL	r0x1033
	SUBWF	r0x1033,W
	BTFSC	STATUS,0
	GOTO	_00126_DS_
;genSkipc:3080: created from rifx:0x2260cc
;	.line	78; "main.c"	PORTA = bits[i];
	MOVF	r0x1033,W
	ADDLW	(_pattern_bits_1_1 + 0)
	MOVWF	r0x1034
	MOVLW	high (_pattern_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x1036
	MOVF	r0x1034,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x1036,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTA
	MOVWF	_PORTA
;	.line	79; "main.c"	PORTB = bits[i+1];
	BANKSEL	r0x1033
	INCF	r0x1033,W
	MOVWF	r0x1034
	ADDLW	(_pattern_bits_1_1 + 0)
	MOVWF	r0x1034
	MOVLW	high (_pattern_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x1037
	MOVF	r0x1034,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x1037,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTB
	MOVWF	_PORTB
;	.line	80; "main.c"	delay(500);
	MOVLW	0xf4
	PAGESEL	_delay
	CALL	_delay
	PAGESEL	$
;	.line	76; "main.c"	for(i = 20; i < 30; i=i+2) 
	BANKSEL	r0x1033
	INCF	r0x1033,F
	INCF	r0x1033,F
	GOTO	_00123_DS_
_00126_DS_
;	.line	83; "main.c"	for(i = 38; i < 42; i=i+2) 
	MOVLW	0x26
	BANKSEL	r0x1033
	MOVWF	r0x1033
;unsigned compare: left < lit(0x2A=42), size=1
_00127_DS_
	MOVLW	0x2a
	BANKSEL	r0x1033
	SUBWF	r0x1033,W
	BTFSC	STATUS,0
	GOTO	_00130_DS_
;genSkipc:3080: created from rifx:0x2260cc
;	.line	85; "main.c"	PORTA = bits[i];
	MOVF	r0x1033,W
	ADDLW	(_pattern_bits_1_1 + 0)
	MOVWF	r0x1034
	MOVLW	high (_pattern_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x1037
	MOVF	r0x1034,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x1037,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTA
	MOVWF	_PORTA
;	.line	86; "main.c"	PORTB = bits[i+1];
	BANKSEL	r0x1033
	INCF	r0x1033,W
	MOVWF	r0x1034
	ADDLW	(_pattern_bits_1_1 + 0)
	MOVWF	r0x1034
	MOVLW	high (_pattern_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x1038
	MOVF	r0x1034,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x1038,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTB
	MOVWF	_PORTB
;	.line	87; "main.c"	delay(500);
	MOVLW	0xf4
	PAGESEL	_delay
	CALL	_delay
	PAGESEL	$
;	.line	83; "main.c"	for(i = 38; i < 42; i=i+2) 
	BANKSEL	r0x1033
	INCF	r0x1033,F
	INCF	r0x1033,F
	GOTO	_00127_DS_
_00130_DS_
;	.line	90; "main.c"	for(i = 44; i < 48; i=i+2) 
	MOVLW	0x2c
	BANKSEL	r0x1033
	MOVWF	r0x1033
;unsigned compare: left < lit(0x30=48), size=1
_00131_DS_
	MOVLW	0x30
	BANKSEL	r0x1033
	SUBWF	r0x1033,W
	BTFSC	STATUS,0
	GOTO	_00135_DS_
;genSkipc:3080: created from rifx:0x2260cc
;	.line	92; "main.c"	PORTA = bits[i];
	MOVF	r0x1033,W
	ADDLW	(_pattern_bits_1_1 + 0)
	MOVWF	r0x1034
	MOVLW	high (_pattern_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x1038
	MOVF	r0x1034,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x1038,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTA
	MOVWF	_PORTA
;	.line	93; "main.c"	PORTB = bits[i+1];
	BANKSEL	r0x1033
	INCF	r0x1033,W
	MOVWF	r0x1034
	ADDLW	(_pattern_bits_1_1 + 0)
	MOVWF	r0x1034
	MOVLW	high (_pattern_bits_1_1 + 0)
	BTFSC	STATUS,0
	ADDLW	0x01
	MOVWF	r0x1039
	MOVF	r0x1034,W
	MOVWF	FSR
	BCF	STATUS,7
	BTFSC	r0x1039,0
	BSF	STATUS,7
	MOVF	INDF,W
	BANKSEL	_PORTB
	MOVWF	_PORTB
;	.line	94; "main.c"	delay(500);
	MOVLW	0xf4
	PAGESEL	_delay
	CALL	_delay
	PAGESEL	$
;	.line	90; "main.c"	for(i = 44; i < 48; i=i+2) 
	BANKSEL	r0x1033
	INCF	r0x1033,F
	INCF	r0x1033,F
	GOTO	_00131_DS_
_00135_DS_
	RETURN	
; exit point of _pattern

;***
;  pBlock Stats: dbName = C
;***
;entry:  _put_byte	;Function start
; 2 exit points
;has an exit
;3 compiler assigned registers:
;   r0x1033
;   STK00
;   r0x1034
;; Starting pCode block
_put_byte	;Function start
; 2 exit points
;;1	MOVWF	r0x1033
;	.line	29; "main.c"	void put_byte(int i)
	MOVF	STK00,W
	BANKSEL	r0x1034
	MOVWF	r0x1034
_00112_DS_
;	.line	31; "main.c"	while(!TRMT)
	BANKSEL	_TXSTA_bits
	BTFSC	_TXSTA_bits,1
	GOTO	_00114_DS_
	nop 
	GOTO	_00112_DS_
_00114_DS_
;	.line	35; "main.c"	TXREG=i;	// Transmit		
	BANKSEL	r0x1034
	MOVF	r0x1034,W
	BANKSEL	_TXREG
	MOVWF	_TXREG
	RETURN	
; exit point of _put_byte

;***
;  pBlock Stats: dbName = C
;***
;entry:  _get_byte	;Function start
; 2 exit points
;has an exit
;3 compiler assigned registers:
;   r0x1033
;   r0x1034
;   STK00
;; Starting pCode block
_get_byte	;Function start
; 2 exit points
_00105_DS_
;	.line	20; "main.c"	while(!RCIF)  // wait for character
	BANKSEL	_PIR1_bits
	BTFSC	_PIR1_bits,5
	GOTO	_00107_DS_
	nop 
	GOTO	_00105_DS_
_00107_DS_
;	.line	25; "main.c"	i= RCREG;
	BANKSEL	_RCREG
	MOVF	_RCREG,W
;	.line	26; "main.c"	return i;
	BANKSEL	r0x1033
	MOVWF	r0x1033
	MOVWF	STK00
	MOVLW	0x00
	RETURN	
; exit point of _get_byte


;	code size estimation:
;	  489+   91 =   580 instructions ( 1342 byte)

	end
