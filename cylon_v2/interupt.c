#include "interupt.h"

/*
   init_hw() - initialize everything

  A1 A0 A7 A6 V+ B7 B6 B5 B4 
  |  |  |  |  |  |  |  |  |
 ---------------------------
 |      PIC 16F648         |
 -o-------------------------
  |  |  |  |  |  |  |  |  |
  A2 A3 A4 A5 G  B0 B1 B2 B3
 
 
 B0-B7 A0-A3 Are connected to LED
 B1 RX
 B2 TX
 A4 Sound output
 
 A5/A6/A7 un assigned
 
*/


unsigned char Msec;
unsigned char wlc;  		//wave length in 0.5us i.e. 1Khz = 0.5us on, 0.5us off -> L=1ms => 1Khz


void init_hw(void) {
	
	CMCON = 0x07;           /* disable comparators */
    T0CS = 1;               /* clear to enable timer mode */
    PSA = 1;                /* clear to assign prescaller to TMRO */
    
    /*
    The TMR0 interupt will occur when TMR0 overflows from 0xFF to
    0x00.  Without a prescaler, TMR0 will increment every clock
    cycle resulting in an interrupt every 256 cycles.  However, 
    using a prescaler, we can force that interrupt to occure at
    less frequent intervals.
    
    Each clock cycle is 1/4 the external clock.  Using that, and
    knowing the prescaler, we can determine the time interval for
    our interrupt.  
    
    PS2 PS1 PS0 Ratio   Cycles  4MHz        10MHz
    0   0   0   1:2     512      512.0 uS    204.8 uS    
    0   0   1   1:4     1024     1.024 mS    409.6 uS
    0   1   0   1:8     2048     2.048 mS    819.2 uS
    0   1   1   1:16    4096     4.096 mS    1.638 mS
    1   0   0   1:32    8192     8.192 mS    3.276 mS
    1   0   1   1:64    16384   16.384 mS    6.553 mS
    1   1   0   1:128   32768   32.768 mS   13.107 mS
    1   1   1   1:256   65536   65.536 mS   26.214 mS 
    */
    
    PS2 = 0;                /* 011 @ 4Mhz = 1.638 mS */
    PS1 = 0;  
    PS0 = 0;  

    INTCON = 0;             /* clear interrupt flag bits */
    GIE = 1;                /* global interrupt enable */
    T0IE = 1;               /* TMR0 overflow interrupt enable */
            
    TMR0 = 0;               /* clear the value in TMR0 */

	SPBRG=SPBRG_VALUE;	// Baud Rate register, calculated by macro
	BRGH=BAUD_HI;

	SYNC=0;			// Disable Synchronous/Enable Asynchronous
	SPEN=1;			// Enable serial port
	TXEN=1;			// Enable transmission mode
	CREN=1;			// Enable reception mode
	
	TRISA= 0x00; 			// all outputs		
	TRISB= 0x00; 			// all outputs		
	TRISB|=RX_BIT;	// These need to be 1 for USART to work
	
	PORTA=0;
	PORTB=0;
	

}

static void isr(void) interrupt 0 { 
    T0IF = 0;               // Clear timer interrupt flag
		
	if (Msec >0) Msec--;
		
}

// ------------------------------------------------
// a simple delay function
// ------------------------------------------------

#define USE_ASSEMBLY (1)
static unsigned char delayus_variable;

#if !USE_ASSEMBLY

void delay_us(int us)
{
	delayus_variable=(unsigned char)(us<<5); \
	while (delayus_variable != 0)
	{
		delayus_variable--;
	}
}

#else

void delay_us(int us)
{
	delayus_variable=(unsigned char)(us/4); \
	_asm
		;movlb (_delayus_variable) >> 8
		BANKSEL	_delayus_variable        
		nop
		;decfsz (_delayus_variable)&0ffh,f
		decfsz _delayus_variable,f
		goto $ - 2
	_endasm;
}

#endif

void delay(unsigned char ms)
{
	
	while (ms>0) 
	{
		delay_us(1000);
		ms--;
	}  		
}
