;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 2.9.2 #5492 (Aug 17 2009) (MINGW32)
; This file was generated Wed Sep 09 09:56:28 2009
;--------------------------------------------------------
; PIC port for the 14-bit core
;--------------------------------------------------------
;	.file	"sound.c"
	list	p=16f648A
	radix dec
	include "p16f648A.inc"
;--------------------------------------------------------
; external declarations
;--------------------------------------------------------
	extern	_init_hw
	extern	_delay_us
	extern	_delay
	extern	__mulint
	extern	__divsint
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
	extern	_wlc
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
	global	_play_tone
	global	_s_mask
	global	_s_bytes
	global	_s_bytes_
	global	_c_byte

;--------------------------------------------------------
; global definitions
;--------------------------------------------------------
UD_sound_0	udata
_s_mask	res	1

UD_sound_1	udata
_s_bytes	res	3

UD_sound_2	udata
_s_bytes_	res	3

UD_sound_3	udata
_c_byte	res	1

;--------------------------------------------------------
; absolute symbol definitions
;--------------------------------------------------------
;--------------------------------------------------------
; compiler-defined variables
;--------------------------------------------------------
UDL_sound_0	udata
r0x1001	res	1
r0x1000	res	1
r0x1003	res	1
r0x1002	res	1
r0x1004	res	1
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
code_sound	code
;***
;  pBlock Stats: dbName = C
;***
;entry:  _play_tone	;Function start
; 2 exit points
;has an exit
;functions called:
;   __mulint
;   __divsint
;   __divsint
;   _delay_us
;   _delay_us
;   __mulint
;   __divsint
;   __divsint
;   _delay_us
;   _delay_us
;9 compiler assigned registers:
;   r0x1000
;   STK00
;   r0x1001
;   STK01
;   r0x1002
;   STK02
;   r0x1003
;   r0x1004
;   r0x1005
;; Starting pCode block
_play_tone	;Function start
; 2 exit points
;	.line	67; "sound.c"	void play_tone(int delay, int duration) {
	BANKSEL	r0x1000
	MOVWF	r0x1000
	MOVF	STK00,W
	MOVWF	r0x1001
	MOVF	STK01,W
	MOVWF	r0x1002
	MOVF	STK02,W
;	.line	77; "sound.c"	int tmp = 100 * duration;
	MOVWF	r0x1003
	MOVWF	STK02
	MOVF	r0x1002,W
	MOVWF	STK01
	MOVLW	0x64
	MOVWF	STK00
	MOVLW	0x00
	PAGESEL	__mulint
	CALL	__mulint
	PAGESEL	$
	BANKSEL	r0x1002
	MOVWF	r0x1002
	MOVF	STK00,W
	MOVWF	r0x1003
;	.line	78; "sound.c"	int delaysm = delay / 50;
	MOVLW	0x32
	MOVWF	STK02
	MOVLW	0x00
	MOVWF	STK01
	MOVF	r0x1001,W
	MOVWF	STK00
	MOVF	r0x1000,W
	PAGESEL	__divsint
	CALL	__divsint
	PAGESEL	$
	BANKSEL	r0x1004
	MOVWF	r0x1004
	MOVF	STK00,W
;;1	MOVWF	r0x1005
;	.line	79; "sound.c"	int cycles = tmp / delaysm;
	MOVWF	STK02
	MOVF	r0x1004,W
	MOVWF	STK01
	MOVF	r0x1003,W
	MOVWF	STK00
	MOVF	r0x1002,W
	PAGESEL	__divsint
	CALL	__divsint
	PAGESEL	$
	BANKSEL	r0x1002
	MOVWF	r0x1002
	MOVF	STK00,W
	MOVWF	r0x1003
;swapping arguments (AOP_TYPEs 1/2)
;signed compare: left >= lit(0x1=1), size=2, mask=ffff
_00105_DS_
;	.line	81; "sound.c"	while(cycles > 0) {
	BANKSEL	r0x1002
	MOVF	r0x1002,W
	ADDLW	0x80
	ADDLW	0x80
	BTFSS	STATUS,2
	GOTO	_00113_DS_
	MOVLW	0x01
	SUBWF	r0x1003,W
_00113_DS_
	BTFSS	STATUS,0
	GOTO	_00108_DS_
;genSkipc:3080: created from rifx:0x2260cc
;	.line	82; "sound.c"	PORTA ^= 0X10; 
	MOVLW	0x10
	BANKSEL	_PORTA
	XORWF	_PORTA,F
;	.line	83; "sound.c"	delay_us(delay);
	BANKSEL	r0x1001
	MOVF	r0x1001,W
	MOVWF	STK00
	MOVF	r0x1000,W
	PAGESEL	_delay_us
	CALL	_delay_us
	PAGESEL	$
;	.line	84; "sound.c"	PORTA ^= 0X10; 
	MOVLW	0x10
	BANKSEL	_PORTA
	XORWF	_PORTA,F
;	.line	85; "sound.c"	delay_us(delay);
	BANKSEL	r0x1001
	MOVF	r0x1001,W
	MOVWF	STK00
	MOVF	r0x1000,W
	PAGESEL	_delay_us
	CALL	_delay_us
	PAGESEL	$
;	.line	86; "sound.c"	cycles--;
	MOVLW	0xff
	BANKSEL	r0x1003
	ADDWF	r0x1003,F
	BTFSS	STATUS,0
	DECF	r0x1002,F
	GOTO	_00105_DS_
_00108_DS_
	RETURN	
; exit point of _play_tone


;	code size estimation:
;	   64+   20 =    84 instructions (  208 byte)

	end
