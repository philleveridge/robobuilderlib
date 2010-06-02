;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 2.9.2 #5492 (Aug 17 2009) (MINGW32)
; This file was generated Wed Sep 09 13:57:17 2009
;--------------------------------------------------------
; PIC port for the 14-bit core
;--------------------------------------------------------
;	.file	"interupt.c"
	list	p=16f648A
	radix dec
	include "p16f648A.inc"
;--------------------------------------------------------
; external declarations
;--------------------------------------------------------
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
	extern	___sdcc_saved_fsr

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
	global	_delay
	global	_delay_us
	global	_init_hw
	global	_Msec
	global	_wlc

;--------------------------------------------------------
; global definitions
;--------------------------------------------------------
UD_interupt_0	udata
_Msec	res	1

UD_interupt_1	udata
_wlc	res	1

;--------------------------------------------------------
; absolute symbol definitions
;--------------------------------------------------------
;--------------------------------------------------------
; compiler-defined variables
;--------------------------------------------------------
UDL_interupt_0	udata
r0x1004	res	1
r0x1003	res	1
r0x1005	res	1
_delayus_variable	res	1
;--------------------------------------------------------
; initialized data
;--------------------------------------------------------
;--------------------------------------------------------
; overlayable items in internal ram 
;--------------------------------------------------------
;	udata_ovr
;--------------------------------------------------------
; interrupt and initialization code
;--------------------------------------------------------
c_interrupt	code	0x4
__sdcc_interrupt
;***
;  pBlock Stats: dbName = I
;***
;entry:  _isr	;Function start
; 0 exit points
;; Starting pCode block
_isr	;Function start
; 0 exit points
;	.line	85; "interupt.c"	static void isr(void) interrupt 0 { 
	MOVWF	WSAVE
	SWAPF	STATUS,W
	CLRF	STATUS
	MOVWF	SSAVE
	MOVF	PCLATH,W
	CLRF	PCLATH
	MOVWF	PSAVE
	MOVF	FSR,W
	BANKSEL	___sdcc_saved_fsr
	MOVWF	___sdcc_saved_fsr
;	.line	86; "interupt.c"	T0IF = 0;               // Clear timer interrupt flag
	BANKSEL	_INTCON_bits
	BCF	_INTCON_bits,2
;	.line	88; "interupt.c"	if (Msec >0) Msec--;
	MOVLW	0x00
	BANKSEL	_Msec
	IORWF	_Msec,W
	BTFSS	STATUS,2
	DECF	_Msec,F
	BANKSEL	___sdcc_saved_fsr
	MOVF	___sdcc_saved_fsr,W
	MOVWF	FSR
	MOVF	PSAVE,W
	MOVWF	PCLATH
	CLRF	STATUS
	SWAPF	SSAVE,W
	MOVWF	STATUS
	SWAPF	WSAVE,F
	SWAPF	WSAVE,W
END_OF_INTERRUPT
	RETFIE	

;--------------------------------------------------------
; code
;--------------------------------------------------------
code_interupt	code
;***
;  pBlock Stats: dbName = C
;***
;entry:  _delay	;Function start
; 2 exit points
;has an exit
;functions called:
;   _delay_us
;   _delay_us
;2 compiler assigned registers:
;   r0x1005
;   STK00
;; Starting pCode block
_delay	;Function start
; 2 exit points
;	.line	127; "interupt.c"	void delay(unsigned char ms)
	BANKSEL	r0x1005
	MOVWF	r0x1005
_00120_DS_
;	.line	130; "interupt.c"	while (ms>0) 
	MOVLW	0x00
	BANKSEL	r0x1005
	IORWF	r0x1005,W
	BTFSC	STATUS,2
	GOTO	_00123_DS_
;	.line	132; "interupt.c"	delay_us(1000);
	MOVLW	0xe8
	MOVWF	STK00
	MOVLW	0x03
	CALL	_delay_us
;	.line	133; "interupt.c"	ms--;
	BANKSEL	r0x1005
	DECF	r0x1005,F
	GOTO	_00120_DS_
_00123_DS_
	RETURN	
; exit point of _delay

;***
;  pBlock Stats: dbName = C
;***
;entry:  _delay_us	;Function start
; 2 exit points
;has an exit
;functions called:
;   __divsint
;   __divsint
;5 compiler assigned registers:
;   r0x1003
;   STK00
;   r0x1004
;   STK02
;   STK01
;; Starting pCode block
_delay_us	;Function start
; 2 exit points
;	.line	112; "interupt.c"	void delay_us(int us)
	BANKSEL	r0x1003
	MOVWF	r0x1003
	MOVF	STK00,W
	MOVWF	r0x1004
;	.line	114; "interupt.c"	delayus_variable=(unsigned char)(us/4); \
	MOVLW	0x04
	MOVWF	STK02
	MOVLW	0x00
	MOVWF	STK01
	MOVF	r0x1004,W
	MOVWF	STK00
	MOVF	r0x1003,W
	PAGESEL	__divsint
	CALL	__divsint
	PAGESEL	$
	BANKSEL	r0x1003
	MOVWF	r0x1003
	MOVF	STK00,W
	MOVWF	r0x1004
	BANKSEL	_delayus_variable
	MOVWF	_delayus_variable
	;movlb (_delayus_variable) >> 8
	BANKSEL _delayus_variable
	nop
	;decfsz (_delayus_variable)&0ffh,f
	decfsz _delayus_variable,f
	goto $ - 2
	
	RETURN	
; exit point of _delay_us

;***
;  pBlock Stats: dbName = C
;***
;entry:  _init_hw	;Function start
; 2 exit points
;has an exit
;; Starting pCode block
_init_hw	;Function start
; 2 exit points
;	.line	31; "interupt.c"	CMCON = 0x07;           /* disable comparators */
	MOVLW	0x07
	BANKSEL	_CMCON
	MOVWF	_CMCON
;	.line	32; "interupt.c"	T0CS = 1;               /* clear to enable timer mode */
	BANKSEL	_OPTION_REG_bits
	BSF	_OPTION_REG_bits,5
;	.line	33; "interupt.c"	PSA = 1;                /* clear to assign prescaller to TMRO */
	BSF	_OPTION_REG_bits,3
;	.line	57; "interupt.c"	PS2 = 0;                /* 011 @ 4Mhz = 1.638 mS */
	BCF	_OPTION_REG_bits,2
;	.line	58; "interupt.c"	PS1 = 0;  
	BCF	_OPTION_REG_bits,1
;	.line	59; "interupt.c"	PS0 = 0;  
	BCF	_OPTION_REG_bits,0
;	.line	61; "interupt.c"	INTCON = 0;             /* clear interrupt flag bits */
	BANKSEL	_INTCON
	CLRF	_INTCON
;	.line	62; "interupt.c"	GIE = 1;                /* global interrupt enable */
	BSF	_INTCON_bits,7
;	.line	63; "interupt.c"	T0IE = 1;               /* TMR0 overflow interrupt enable */
	BSF	_INTCON_bits,5
;	.line	65; "interupt.c"	TMR0 = 0;               /* clear the value in TMR0 */
	CLRF	_TMR0
;	.line	67; "interupt.c"	SPBRG=SPBRG_VALUE;	// Baud Rate register, calculated by macro
	MOVLW	0x19
	BANKSEL	_SPBRG
	MOVWF	_SPBRG
;	.line	68; "interupt.c"	BRGH=BAUD_HI;
	BSF	_TXSTA_bits,2
;	.line	70; "interupt.c"	SYNC=0;			// Disable Synchronous/Enable Asynchronous
	BCF	_TXSTA_bits,4
;	.line	71; "interupt.c"	SPEN=1;			// Enable serial port
	BANKSEL	_RCSTA_bits
	BSF	_RCSTA_bits,7
;	.line	72; "interupt.c"	TXEN=1;			// Enable transmission mode
	BANKSEL	_TXSTA_bits
	BSF	_TXSTA_bits,5
;	.line	73; "interupt.c"	CREN=1;			// Enable reception mode
	BANKSEL	_RCSTA_bits
	BSF	_RCSTA_bits,4
;	.line	75; "interupt.c"	TRISA= 0x00; 			// all outputs		
	BANKSEL	_TRISA
	CLRF	_TRISA
;	.line	76; "interupt.c"	TRISB= 0x00; 			// all outputs		
	CLRF	_TRISB
;	.line	77; "interupt.c"	TRISB|=RX_BIT;	// These need to be 1 for USART to work
	BSF	_TRISB,1
;	.line	79; "interupt.c"	PORTA=0;
	BANKSEL	_PORTA
	CLRF	_PORTA
;	.line	80; "interupt.c"	PORTB=0;
	CLRF	_PORTB
	RETURN	
; exit point of _init_hw


;	code size estimation:
;	   76+   21 =    97 instructions (  236 byte)

	end
