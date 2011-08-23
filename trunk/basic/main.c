//==============================================================================
//	 Basic - for robobuilder
//	 
//   2010 June l3v3rz 
//==============================================================================
#include <avr/io.h>

// Standard Input/Output functions
#include <stdio.h>
#include "main.h"

#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "macro.h"
#include "math.h"
#include "uart.h"
#include "rprintf.h"
#include "adc.h"
#include "ir.h"
#include "accelerometer.h"
#include "wckmotion.h"

#define BR115200			7 
#define DATA_REGISTER_EMPTY (1<<UDRE)


//adc.c
extern BYTE				gAD_Ch_Index;
extern void 			ADC_set(BYTE );
extern volatile BYTE   	MIC_SAMPLING;

// global variables------------------------------------------------------------
WORD    gBtnCnt;						// counter for PF button press

int		gX,gY,gZ;
int 	autobalance;
int     response;						//reponsd to event on/off

// timer variables----------------------------------------------------------------

BYTE 	F_PS_PLUGGED;
BYTE 	F_CHARGING;

volatile WORD   gPSplugCount;
volatile WORD   gPSunplugCount;
volatile WORD	gPwrLowCount;

volatile WORD    g10MSEC;
volatile WORD    gMSEC;
volatile BYTE    gSEC;
volatile BYTE    gMIN;
volatile BYTE    gHOUR;

volatile WORD	gtick;
volatile WORD	mstimer;
volatile WORD	gSEC_DCOUNT;
volatile WORD	gMIN_DCOUNT;

void heart()
{
	// the beating heart ...
	// 1000 ms 
	// 0-200 ms PF1 LED ON  200-400 ms PF2 LED on
	// else off
	if (gMSEC<200)
	{
		PF1_LED1_ON;    
		PF1_LED2_OFF;
		PF2_LED_OFF;
	}
	else if (gMSEC>200 && gMSEC<400)
	{
		PF1_LED1_OFF;    
		PF1_LED2_OFF;
		PF2_LED_ON;
	}
	else
	{
		PF1_LED1_OFF;    
		PF1_LED2_OFF;
		PF2_LED_OFF;
	}
}

void delay_ms(int m)
{
	mstimer=m;
	while (mstimer>0) {}; //wait for mstimer to clear;
}

extern volatile BYTE MIC_RATE;

//------------------------------------------------------------------------------
// Timer0 one millisec clock interrupt routine (0.998ms)
//------------------------------------------------------------------------------
ISR(TIMER0_OVF_vect)
{
	TCNT0 = 25; 		//reset the timer
	
	// 1ms 
	gtick++;
	
	if (mstimer>0) mstimer--;
	
	heart(); // beating
	
	if (MIC_SAMPLING && gMSEC%MIC_RATE==0)
	{
		//every 4ms
		gAD_Ch_Index = MIC_CH;		
		ADC_set(ADC_MODE_SINGLE);
	}
	
    	if(++gMSEC>999){
		// 1s 
        gMSEC=0;
		
        if(gSEC_DCOUNT > 0)	gSEC_DCOUNT--;

        if(++gSEC>59){
			// 1m 
            gSEC=0;
			if(gMIN_DCOUNT > 0)	gMIN_DCOUNT--;
			
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
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
void DetectPower(void)
{
	if(F_PS_PLUGGED){
		if(gVOLTAGE >= U_T_OF_POWER)
			gPSunplugCount = 0;
		else
			gPSunplugCount++;
		if(gPSunplugCount > 6){
			F_PS_PLUGGED = 0;
			gPSunplugCount = 0;
		}
	}
	else{
		if(gVOLTAGE >= U_T_OF_POWER){
			gPSunplugCount = 0;
			gPSplugCount++;
		}
		else{
			gPSplugCount = 0;
		}

		if(gPSplugCount>2){
			F_PS_PLUGGED = 1;
			gPSplugCount = 0;
		}
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ChargeNiMH(void)
{
	F_CHARGING = 1;
	gMIN_DCOUNT = 5;
	rprintf("Start .. %dmV\r\n",gVOLTAGE);
	while(gMIN_DCOUNT)
	{
		if (uartGetByte()>=0 || irGetByte()>=0)
  			break;

	    	if (gSEC%5==0)
		{
			rprintf("%d:%d short 40ms charge pulses %dmV\r\n", gMIN, gSEC,gVOLTAGE);
		}
		PWR_LED2_OFF;
		PWR_LED1_ON;
		Get_VOLTAGE();	DetectPower();
		if(F_PS_PLUGGED == 0) break;
		CHARGE_ENABLE;
		delay_ms(40);
		CHARGE_DISABLE;
		delay_ms(500-40);
		PWR_LED1_OFF;
		Get_VOLTAGE();	DetectPower();
		if(F_PS_PLUGGED == 0) break;
		delay_ms(500);
	}
	gMIN_DCOUNT = 85;
	while(gMIN_DCOUNT)
	{
		if (uartGetByte()>=0 || irGetByte()>=0)
  			break;

		PWR_LED2_OFF;
		if(g10MSEC > 500)	
			PWR_LED1_ON;
		else			
			PWR_LED1_OFF;
		if(g10MSEC == 0 || g10MSEC == 500)
		{
			Get_VOLTAGE();
			DetectPower();
			if (gSEC%5==0)
			{
				rprintf ("%d:%d full charge power %dmV\r\n", gMIN,gSEC,gVOLTAGE);
			}
		}
		if(F_PS_PLUGGED == 0) break;
		CHARGE_ENABLE;
	}
	CHARGE_DISABLE;
	F_CHARGING = 0;
	rprintf ("Done %dmV\r\n", gVOLTAGE);
}

#define PF1_BTN_PRESSED 1
#define PF2_BTN_PRESSED 3

BYTE	gBtn_val;
WORD	gBtnCnt;


//------------------------------------------------------------------------------
//  Check Routine
//------------------------------------------------------------------------------
void ReadButton(void)
{
	if((PINA&03) == 1)  // PF1 on, PF2 off
	{
		delay_ms(10);
		if((PINA&1) == 1)
		{
			gBtn_val = PF1_BTN_PRESSED;
		}
	}
	if((PINA&3) == 2)  // PF1 on, PF2 off
	{
		delay_ms(10);
		if((PINA&3) == 2)
		{
			gBtn_val = PF2_BTN_PRESSED;
		}
	}
} 


//-----------------------------------------------------------------------------
// Process routine
//-----------------------------------------------------------------------------


void chargemode()
{
	int cnt;
	rprintf("charge mode \r\n");

	for (cnt=0; cnt<10; cnt++)
	{
		PWR_LED2_ON;	// RED on
		delay_ms(50);
		Get_VOLTAGE();
		DetectPower();
		rprintf("%d mV\r\n", gVOLTAGE);
		PWR_LED2_OFF;   // RED off
		delay_ms(50);
	}

	if(F_PS_PLUGGED)
	{
		rprintf("Plugged in - charging\r\n");
		wckPowerDown();		// put servo in breakmode (power off)
		ChargeNiMH();  			//initiate battery charging
		rprintf("Complete\r\n");
	}
	else
	{
		rprintf("Not plugged in\r\n");
		PWR_LED2_ON	;
	}
}

void ProcButton(void)
{
	int cnt;
	if(gBtn_val == PF1_BTN_PRESSED)
	{
		chargemode();
	}
}


//------------------------------------------------------------------------------
// Initialise Ports
//------------------------------------------------------------------------------
void HW_init(void)
{
	// Input/Output Ports initialisation
	// Port A initialisation
	// Func7=Out Func6=Out Func5=Out Func4=Out Func3=Out Func2=Out Func1=In Func0=In 
	// State7=0 State6=0 State5=0 State4=0 State3=0 State2=0 State1=P State0=P 
	PORTA=0x03;
	DDRA=0xFC;

	// Port B initialisation
	// Func7=In Func6=Out Func5=Out Func4=Out Func3=In Func2=Out Func1=In Func0=In 
	// State7=T State6=0 State5=0 State4=0 State3=T State2=0 State1=T State0=T 
	PORTB=0x00;
	DDRB=0x74;

	// Port C initialisation
	// Func7=Out Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
	// State7=0 State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
	PORTC=0x00;
	DDRC=0x80;

	// Port D initialisation
	// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
	// State7=T State6=T State5=T State4=T State3=T State2=T State1=P State0=P 
	PORTD=0x83;
	DDRD=0x80;

	// Port E initialisation
	// Func7=In Func6=In Func5=In Func4=In Func3=Out Func2=In Func1=In Func0=In 
	// State7=T State6=P State5=P State4=P State3=0 State2=T State1=T State0=T 
	PORTE=0x30; // 0x70;
	DDRE=0x08;

	// Port F initialisation
	// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
	// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
	PORTF=0x00;
	DDRF=0x00;

	// Port G initialisation
	// Func4=In Func3=In Func2=Out Func1=In Func0=In 
	// State4=T State3=T State2=0 State1=T State0=T 
	PORTG=0x00;
	DDRG=0x04;

	// Timer 0---------------------------------------------------------------
	// Millisecond Clock
	// Timer/Counter 0 initialisation
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
	// Timer/Counter 1 initialisation
	// Clock source: System Clock
	// Clock value: 14.400 kHz
	// Clock Period = 1/14400 = 69.4us
	// Mode: Normal top=FFFFh
	// OC1A output: Discon.
	// OC1B output: Discon.
	// OC1C output: Discon.
	// Noise Canceller: Off
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
	// Timer/Counter 2 initialisation
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

	// External Interrupt(s) initialisation
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

	// Timer(s)/Counter(s) Interrupt(s) initialisation
	TIMSK=0x00;
	ETIMSK=0x00;

	// USART0 initialisation
	// Communication Parameters: 8 Data, 1 Stop, No Parity
	// USART0 Receiver: Off
	// USART0 Transmitter: On
	// USART0 Mode: Asynchronous
	// USART0 Baud rate: 115200
	UCSR0A=0x00;
	//UCSR0B=0x98;
	//UCSR0B=0x48;
	UCSR0B = (1<<RXEN)|(1<<TXEN); //enable reads for GetPos !!
	UCSR0C=0x06;
	UBRR0H=0x00;
	UBRR0L=0x07;

	// USART1 initialisation
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

	// Analog Comparator initialisation
	// Analog Comparator: Off
	// Analog Comparator Input Capture by Timer/Counter 1: Off
	// Analog Comparator Output: Off
	ACSR=0x80;
	SFIOR=0x00;

    //ADC initialisation
    //ADC Clock frequency: 460.800 kHz
    //ADC Voltage Reference: AREF pin
    //Only the 8 most significant bits of
    //the AD conversion result are used
    ADMUX=ADC_VREF_TYPE;
    ADCSRA=0x00;	

	TWCR = 0;
}

//------------------------------------------------------------------------------
// Initialise software states
//------------------------------------------------------------------------------
void SW_init(void)
{
	PF1_LED1_OFF;			// LED states
	PF1_LED2_OFF;
	PF2_LED_OFF;
	PWR_LED1_OFF;
	PWR_LED2_OFF;
	RUN_LED1_OFF;
	RUN_LED2_OFF;
	ERR_LED_OFF;

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
extern void basic();
	
int main(void) 
{
	HW_init();					// Initialise ATMega Ports
	SW_init();					// Initialise software states
	uartInit();					// initialize UART (serial port)
	uartSetBaudRate(115200);	// set UART speed to 115200 baud
	rprintfInit(uartSendByte);  // configure rprintf to use UART for output
			
	sei();						// enable interrupts
	TIMSK |= 0x01;				// Timer0 Overflow Interrupt enable
	
	PWR_LED1_ON; 				// Power green light on
	
	Acc_init();				    // initialise acceleromter
	
	ReadButton();	
	ProcButton();
		
	basic();
	
	while (1) { }

}
