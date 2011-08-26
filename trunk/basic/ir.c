// ir.c
//
//	Routines for receiving data from the IR controller

#include <avr/interrupt.h>
#include "avrlibtypes.h"
#include "ir.h"
#include "main.h"
#include "macro.h"


// IR Control variables---------------------------------------------------------

volatile BYTE	gIRReady;				// IR message received
volatile BYTE	gIRData;				// Data from IR
volatile BYTE	gIRAddr;				// Data from IR

BYTE	IRTemp[4];
volatile BYTE	IRState;						// state or IR receive
BYTE	IRBit;							// bit counter for IR
BYTE	IRByte;							// byte counter for IR (address receipt)
BYTE	pulse_width;


char	gIrBuf[IR_BUFFER_SIZE];
BYTE	gIrBitIndex = 0;

//----------------------------------------------------------------------
// Timer2 IR Timeout interrupt routine
//------------------------------------------------------------------------------
ISR(TIMER2_OVF_vect) 
{
	TIMSK &= 0xBF;			// disable timeout interrupt
	IRState = IR_IDLE;

	PWR_LED1_OFF;
	PWR_LED2_OFF;
	RUN_LED1_OFF;
	RUN_LED2_ON;
}
//------------------------------------------------------------------------------
// External Interrupt 6 IR Remote
//------------------------------------------------------------------------------
ISR(INT6_vect) 
{
	switch(IRState) {

	case IR_IDLE: 
		TCNT2 = 0;				// clear timer 2
		IRState = IR_START;
		TIFR |= 0x40;			// clear overflow
		TIMSK |= 0x40;			// timeout interrupt

		PWR_LED1_ON;

		break;

	case IR_START:
		pulse_width = TCNT2; 	// read timer
		TCNT2 = 0;				// clear timer 2
		if ((pulse_width < IR_START_SHORT) | (pulse_width > IR_START_LONG))
			IRState = IR_IDLE;
		
		else {
			IRBit = 0;
			IRByte = 0;
			IRState = IR_RECEIVE;
			TIFR |= 0x40;			// clear overflow
			TIMSK |= 0x40;			// timeout interrupt
			for (BYTE i=0; i < 4; i++) IRTemp[i] = 0;
			
		}

		PWR_LED1_OFF;	PWR_LED2_ON;
		break;

	case IR_RECEIVE:
		RUN_LED1_ON; PWR_LED1_OFF;	PWR_LED2_OFF;

		pulse_width = TCNT2;    // read time
		TCNT2 = 0;

		if ((pulse_width < IR_BIT_SHORT) | (pulse_width > IR_BIT_LONG))
			IRState = IR_IDLE;
			
		else {
			if (pulse_width > IR_ZERO_ONE)
				IRTemp[IRByte] = (IRTemp[IRByte] >> 1) | 0x80;
			else 
				IRTemp[IRByte] = IRTemp[IRByte] >> 1;
			
			if (++IRBit == 8) {
				IRBit = 0;
				if(++IRByte == 4) {
					gIRAddr = IRTemp[0];
					//if( IRTemp[0] == IR_ADDRESS){     // commented out address checking
						gIRData = IRTemp[3];
						gIRReady = TRUE;
					//}
					IRState = IR_IDLE;
					return;
				}

			}
			TIFR |= 0x40;			// clear overflow
			TIMSK |= 0x40;			// timeout interrupt
		}
		break;
	} // end switch
	
} // end int

//------------------------------------------------------------------------------
// get the next char from the IR receiver, or -1 if none (getchar-style)
//------------------------------------------------------------------------------
int irGetByte(void)
{

	if (!gIRReady) return -1;
	gIRReady = FALSE;
	return gIRData;
}

