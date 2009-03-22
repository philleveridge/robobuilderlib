//==============================================================================
//	 RoboBuilder MainController Sample Program	1.0
//	 2008.04.14	Robobuilder co., ltd.
//   2008.06.30 Richard Ibbotson convert to english and to AVR GCC
//==============================================================================
#include <avr/io.h>

// Standard Input/Output functions
#include <stdio.h>
#include "Main.h"

//#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "global.h"
#include "Macro.h"
#include "Comm.h"
#include "math.h"
#include "uart.h"
#include "rprintf.h"
#include "adc.h"
#include "ir.h"
#include "accelerometer.h"
#include "majormodes.h"

#include <util/delay.h>

volatile extern int		gFrameIdx;	    // frame counter
extern WORD	   	TxInterval;				// Timer 1 interval
extern WORD		gNumOfFrame;

const prog_char version[] = "0.7 - $REV$\r\n";

// software states----------------------------------------------------------------------
volatile BYTE 	F_PLAYING;				// state: playing from Flash


// "major mode" state machine-----------------------------------------------------
int		gNextMode;
extern void idle_mainloop(void);
extern void experimental_mainloop(void);
extern void serialslave_mainloop(void);
extern void charge_mainloop(void);


// global variables------------------------------------------------------------
WORD    gBtnCnt;						// counter for PF button press


int		gX,gY,gZ;
int 	autobalance;
int     response;						//reponsd to event on/off
int	    params[10];						//tuning params

// timer variables----------------------------------------------------------------
volatile WORD    gMSEC;
volatile BYTE    gSEC;
volatile BYTE    gMIN;
volatile BYTE    gHOUR;

// UART variables-----------------------------------------------------------------
volatile BYTE	gTx0Buf[TX0_BUF_SIZE];		// UART0 transmit buffer
volatile BYTE	gTx0Cnt;					// UART0 transmit length
volatile BYTE	gRx0Cnt;					// UART0 receive length
volatile BYTE	gTx0BufIdx;					// UART0 transmit pointer
volatile BYTE	gRx0Buf[RX0_BUF_SIZE];		// UART0 receive buffer


//------------------------------------------------------------------------------
// UART0 wCK Transmit Interrupt Routine
//------------------------------------------------------------------------------
ISR(USART0_TX_vect)
{
	if(gTx0BufIdx<gTx0Cnt){				// is pointer still less than length ?
    	while(!(UCSR0A&(1<<UDRE))); 	// while data register empty
		UDR0=gTx0Buf[gTx0BufIdx];		// send next character
    	gTx0BufIdx++;      				// increment pointer
	}
	else if(gTx0BufIdx==gTx0Cnt){		// all sent
		gTx0BufIdx = 0;					// clear pointer
		gTx0Cnt = 0;					// clear length
	}
}


//------------------------------------------------------------------------------
// UART0 wCK Receive Interrupt Routine
//------------------------------------------------------------------------------
ISR(USART0_RX_vect)
{
	int		i;
    char	data;
	
	data=UDR0;		// get the data
	gRx0Cnt++;		// increment the pointer
    // move the fifo down and store data at top of fifo
   	for(i=1; i<RX0_BUF_SIZE; i++) gRx0Buf[i-1] = gRx0Buf[i];
   	gRx0Buf[RX0_BUF_SIZE-1] = data;

}


//------------------------------------------------------------------------------
// Timer0 one millisec clock interrupt routine (0.998ms)
//------------------------------------------------------------------------------
ISR(TIMER0_OVF_vect)
{
	TCNT0 = 25; 		//reset the timer
	// 1ms 
    if(++gMSEC>999){
		// 1s 
        gMSEC=0;
        if(++gSEC>59){
			// 1m 
            gSEC=0;
            if(++gMIN>59){
				// 1h 
                gMIN=0;
                if(++gHOUR>23)
                    gHOUR=0;
            }
        }
    }
}


//------------------------------------------------------------------------------
// Timer1 Send Frame to wCK interrupt routine
//------------------------------------------------------------------------------
ISR(TIMER1_OVF_vect) 
{
	if (gFrameIdx == gNumOfFrame) {   // are we at the end of the scene ?
		gFrameIdx = 0;
		RUN_LED1_OFF;
		F_PLAYING=0;						// clear F_PLAYING state
		TIMSK &= 0xfb;  					// Timer1 Overflow Interrupt disable
		TCCR1B=0x00;
		return;
	}
	TCNT1=TxInterval;
	TIFR |= 0x04;							// restart timer
	TIMSK |= 0x04;							// Timer1 Overflow Interrupt enable
	MakeFrame();							// build the wCK frame
	SendFrame();							// send the wCK frame
}



//------------------------------------------------------------------------------
// Initialise Ports
//------------------------------------------------------------------------------
void HW_init(void) {
	// Input/Output Ports initialization
	// Port A initialization
	// Func7=Out Func6=Out Func5=Out Func4=Out Func3=Out Func2=Out Func1=In Func0=In 
	// State7=0 State6=0 State5=0 State4=0 State3=0 State2=0 State1=P State0=P 
	PORTA=0x03;
	DDRA=0xFC;

	// Port B initialization
	// Func7=In Func6=Out Func5=Out Func4=Out Func3=In Func2=Out Func1=In Func0=In 
	// State7=T State6=0 State5=0 State4=0 State3=T State2=0 State1=T State0=T 
	PORTB=0x00;
	DDRB=0x74;

	// Port C initialization
	// Func7=Out Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
	// State7=0 State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
	PORTC=0x00;
	DDRC=0x80;

	// Port D initialization
	// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
	// State7=T State6=T State5=T State4=T State3=T State2=T State1=P State0=P 
	PORTD=0x03;
	DDRD=0x00;

	// Port E initialization
	// Func7=In Func6=In Func5=In Func4=In Func3=Out Func2=In Func1=In Func0=In 
	// State7=T State6=P State5=P State4=P State3=0 State2=T State1=T State0=T 
	PORTE=0x70;
	DDRE=0x08;

	// Port F initialization
	// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
	// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
	PORTF=0x00;
	DDRF=0x00;

	// Port G initialization
	// Func4=In Func3=In Func2=Out Func1=In Func0=In 
	// State4=T State3=T State2=0 State1=T State0=T 
	PORTG=0x00;
	DDRG=0x04;

	// Timer 0---------------------------------------------------------------
	// Millisecond Clock
	// Timer/Counter 0 initialization
	// Clock source: System Clock
	// Clock value: 230.400 kHz
	// Clock period = 1/230400 = 4.34us
	// Overflow time = 255*1/230400 = 1.107ms
	// 1ms overflow value =  255-230 = 25
	// Mode: Normal top=FFh
	// OC0 output: Disconnected
	ASSR=0x00;
	TCCR0=0x04;
	TCNT0=0x00;
	OCR0=0x00;

	// Timer 1---------------------------------------------------------------
	// wCK Frame timer
	// Timer/Counter 1 initialization
	// Clock source: System Clock
	// Clock value: 14.400 kHz
	// Clock Period = 1/14400 = 69.4us
	// Mode: Normal top=FFFFh
	// OC1A output: Discon.
	// OC1B output: Discon.
	// OC1C output: Discon.
	// Noise Canceler: Off
	// Input Capture on Falling Edge
	// Timer 1 Overflow Interrupt: On
	// Input Capture Interrupt: Off
	// Compare A Match Interrupt: Off
	// Compare B Match Interrupt: Off
	// Compare C Match Interrupt: Off
	TCCR1A=0x00;
	TCCR1B=0x05;
	TCNT1H=0x00;
	TCNT1L=0x00;
	ICR1H=0x00;
	ICR1L=0x00;
	OCR1AH=0x00;
	OCR1AL=0x00;
	OCR1BH=0x00;
	OCR1BL=0x00;
	OCR1CH=0x00;
	OCR1CL=0x00;

	// Timer 2---------------------------------------------------------------
	// IR Remote timer
	// Timer/Counter 2 initialization
	// Clock source: System Clock
	// Clock freq: 14.400 kHz
	// Clock period = 1/14400 = 69.4us
	// Mode: Normal top=FFh
	// OC2 output: Disconnected
	TCCR2=0x05;
	TCNT2=0x00;
	OCR2=0x00;

	// Timer 3---------------------------------------------------------------
	// Not used
	TCCR3A=0x00;
	TCCR3B=0x03;
	TCNT3H=0x00;
	TCNT3L=0x00;
	ICR3H=0x00;
	ICR3L=0x00;
	OCR3AH=0x00;
	OCR3AL=0x00;
	OCR3BH=0x00;
	OCR3BL=0x00;
	OCR3CH=0x00;
	OCR3CL=0x00;

	// External Interrupt(s) initialization
	// INT0: Off
	// INT1: Off
	// INT2: Off
	// INT3: Off
	// INT4: Off
	// INT5: Off
	// INT6: IR Remote falling edge
	// INT7: Off
	EICRA=0x00;
	EICRB=0x20;
	EIMSK=0x40;

	// Timer(s)/Counter(s) Interrupt(s) initialization
	TIMSK=0x00;
	ETIMSK=0x00;

	// USART0 initialization
	// Communication Parameters: 8 Data, 1 Stop, No Parity
	// USART0 Receiver: Off
	// USART0 Transmitter: On
	// USART0 Mode: Asynchronous
	// USART0 Baud rate: 115200
	UCSR0A=0x00;
	//UCSR0B=0x98;
	//UCSR0B=0x48;
	UCSR0B = (1<<RXEN)|(1<<TXEN) |(1<<TXCIE); //enable reads for GetPos !!
	UCSR0C=0x06;
	UBRR0H=0x00;
	UBRR0L=0x07;

	// USART1 initialization
	// Communication Parameters: 8 Data, 1 Stop, No Parity
	// USART1 Receiver: On
	// USART1 Transmitter: On
	// USART1 Mode: Asynchronous
	// USART1 Baud rate: 115200
	UCSR1A=0x00;
	UCSR1B=0x18;		
	UCSR1C=0x06;
	UBRR1H=0x00;
	UBRR1L=BR115200;	

	TWCR = 0;
}


//------------------------------------------------------------------------------
// Initialise software states
//------------------------------------------------------------------------------
void SW_init(void) {

	PF1_LED1_OFF;			// LED states
	PF1_LED2_OFF;
	PF2_LED_OFF;
	PWR_LED1_OFF;
	PWR_LED2_OFF;
	RUN_LED1_OFF;
	RUN_LED2_OFF;
	ERR_LED_OFF;

	F_PLAYING = 0;          // clear F_Playing

	gTx0Cnt = 0;			// UART0 clear length
	gTx0BufIdx = 0;			// TX0 clear pointer

	PSD_OFF;                // PSD distance sensor off

	gIRReady = FALSE;			// clear IR vales
	IRState = IR_IDLE;

	gMSEC=0;
	gSEC=0;
	gMIN=0;
	gHOUR=0;
}



//------------------------------------------------------------------------------
// Main Routine
//------------------------------------------------------------------------------


void initparams()
{

/* TIMR  0, TLT   1, TORQ  2, PSDL  3
   PSDL2 4, PSDL3 5, MICL  6, MICL2 7
   MICL3 8, VOLT  9
*/	
    params[TIMR] = 20;    // t seconds
	params[TLT]  = 4;     // tlt
	params[TORQ] = 2;     // -
	
#ifdef PSD_SENSOR
	params[PSDL] = 0x40;  // Distance -
	params[PSDL2]= 0x60;  // -
	params[PSDL3]= 0x80;  // -
#else
	params[PSDL] = 0x00;  // Distance -
	params[PSDL2]= 0x00;  // -
	params[PSDL3]= 0x00;  // -
#endif
	
	params[MICL] = 0x79;  // -
	params[MICL2]= 0x99;  // -
	params[MICL2]= 0x99;  // -
	params[VOLT]=  8100;// -voltage level
	
	autobalance=0;	//dynamic blance off
	response=0;		//event response off
}

void pversion()
{
	rprintf("Robos ");
	rprintfProgStr(version);
}

	
int main(void) 
{
	HW_init();					// Initialise ATMega Ports
	SW_init();					// Initialise software states
	uartInit();					// initialize UART (serial port)
	uartSetBaudRate(115200);	// set UART speed to 115200 baud
	rprintfInit(uartSendByte);  // configure rprintf to use UART for output
			
	sei();						// enable interrupts
	TIMSK |= 0x01;				// Timer0 Overflow Interrupt enable
		
	pversion();
	
	PWR_LED1_ON; 				// Power green light on
	
	adc_init();		
	tilt_setup();				// initialise acceleromter
	_delay_ms(200);
	
	initparams();
	
	gNextMode = kIdleMode;
	while (1) {
		switch (gNextMode) {
			case kIdleMode:
				idle_mainloop();
				break;
			case kExperimentalMode:
				experimental_mainloop();
				break;
			case kChargeMode:
				charge_mainloop();
				break;
			case kSerialSlaveMode:
				serialslave_mainloop();
				break;
/*			case kClassicMode:
				classic_mainloop();
				break;
*/
		}
	}
}
