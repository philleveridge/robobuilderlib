//==============================================================================
//	 DCMP - Direct Control Mode Plus
//   Homebre firmware for Robobuilder RBC control unit
//   by l3v3rz
//==============================================================================
#include <avr/io.h>
#include <avr/interrupt.h>

#include "Main.h"
#include "Macro.h"
#include "adc.h"
#include "global.h"
#include "accelerometer.h"

#define BR115200			7 
#define DATA_REGISTER_EMPTY (1<<UDRE)

char *ver = "($Rev$)";

//defined adc.c
extern void Acc_init(void);
extern void AccGetData(void);
extern BYTE sData[];
extern int 	sDcnt;
extern void sample_sound(int);
extern volatile BYTE   MIC_SAMPLING;

//defined femto.c
void printint(int);
void printline(char *c);

//defined battery.c
extern BYTE F_PS_PLUGGED;
extern void delay_ms(int);
extern void BreakModeCmdSend(void);
extern void ChargeNiMH(void);
extern void SelfTest1(void);
extern void DetectPower(void);

//define ir.c
extern int irGetByte(void);


// UART variables-----------------------------------------------------------------

WORD	gRx1Step;
BYTE	gRxData;
WORD	gRx1_DStep;
BYTE	gFileCheckSum;
BYTE	gCMD;
volatile BYTE	PLAY_SOUND;

//------------------------------------------------------------------------------
// UART1 Transmit  Routine
//------------------------------------------------------------------------------

void putByte (BYTE b)
{
	while ( (UCSR1A & DATA_REGISTER_EMPTY) == 0 );
	UDR1 = b;
}

void putch(char c)
{
	putByte(c);
}

//------------------------------------------------------------------------------
// UART0 wCK Receive Interrupt Routine
//------------------------------------------------------------------------------
ISR(USART0_RX_vect) // interrupt [USART0_RXC] void usart0_rx_isr(void)
{
	RUN_LED1_ON;
	
    char	data;	
	data = UDR0;		// get the data
	
	while ( (UCSR1A & DATA_REGISTER_EMPTY) == 0 );
	UDR1 = data;
	return;
}


//------------------------------------------------------------------------------
// UART1 recieve (from PC)
//------------------------------------------------------------------------------
extern volatile BYTE   gSEC;
extern volatile BYTE   gMIN;
extern volatile BYTE   gSoundLevel;
extern void     lights(int n);
extern void     Get_AD_MIC(void);

BYTE 	outb[10];
BYTE 	outbc;
BYTE	gCNT;
BYTE	gAddr;

void SendToSoundIC(BYTE cmd) ;

ISR(USART1_RX_vect) // interrupt [USART1_RXC] void usart1_rx_isr(void)
{
	if (CHK_BIT5(PORTA))  RUN_LED1_ON; else RUN_LED1_OFF; 

    gRxData = UDR1;
	
	while( (UCSR0A & DATA_REGISTER_EMPTY) == 0 );
	UDR0 = gRxData;
		
	if(gRxData == 0xff){
		gRx1_DStep = 1;
		gFileCheckSum = 0;
		return;
	}
	
	switch(gRx1_DStep){
		case 1:
			if(gRxData == 0xBE) // check for 101 11110   (read Position Servo 30)
			{
				gRx1_DStep = 2;
			}
			else 
				gRx1_DStep = 0;
			gFileCheckSum ^= gRxData;
			break;
		case 2:
			gCMD = gRxData;
			if (gCMD==0x0D || gCMD==0x0E)
				gRx1_DStep = 4;
			else			
				gRx1_DStep = 3;
			gFileCheckSum ^= gRxData;
			break;
		case 4:
			// IC2_on and IC2_out
			// gRxData will be numer of bytes to follow
			gAddr=gRxData;
			gRx1_DStep=5;
			break;
		case 5:
			// IC2_on and IC2_out
			// gRxData will be numer of bytes to follow
			gCNT=gRxData;
			outbc=0;
			gRx1_DStep=6;
			gFileCheckSum=0;
			break;
		case 6:
			if (outbc < gCNT)
			{
				gFileCheckSum ^= gRxData;
				outb[outbc] = gRxData;
				outbc++;
			}
			if(outbc==gCNT)
			{
				PF1_LED2_OFF; 
				gRx1_DStep=3;
			}			
			break;
		case 3:
			PF1_LED1_ON;
			if(gRxData == (gFileCheckSum & 0x7f))
			{
				// Depends on gGMD		
				BYTE b1=0, b2=0;
				
				if (gCMD>0x40 && gCMD<(0x40+26))
				{
					PLAY_SOUND = gCMD-0x40;
					gCMD=0;
					return;
				}
				switch (gCMD)
				{
				case 0x01:  
					Acc_GetData();
					b1 = y_value;
					b2 = z_value;
					break;
				case 0x02:
					Acc_GetData();
					b1 = x_value;
					b2 = z_value;	
					break;
				case 0x03:
					PSD_on();
					break;
				case 0x04:
					PSD_off();
					break;
				case 0x05:
					Get_AD_PSD();
					b1 = gDistance;
					break;
				case 0x06:
					Get_VOLTAGE();
					b1 = gVOLTAGE/256;
					b2 = gVOLTAGE%256;
					break;
				case 0x07:
					b1 = irGetByte();
					break;					
				case 0x08:
					sample_sound(1); // on 
					break;
				case 0x09:
					sample_sound(0); // on 
					break;					
				case 0x0A:
					{
					WORD t=0;
					int lc;
					for (lc=0; lc<SDATASZ; lc++) 
					{
						lights(sData[lc]);
						t += sData[lc];  // sum the buffer
						sData[lc]=0;     // and clear
					}
					b1=t/256;
					b2=t%256;
					}
					break;					
				case 0x0B:
					b1 = gSEC;
					b2 = gMIN;
					break;	
				case 0x0C:
					Get_AD_MIC();
					b1 = gSoundLevel;
					lights(b1);
					break;	
				case 0x0D:
					PF2_LED_ON; //yellow
					{
						BYTE inb[10];
						int icnt=outb[0];
						I2C_read (gAddr, gCNT-1, outb+1, icnt, inb);
						for (int k=0; k<icnt; k++)
						{
							putByte(inb[k]);
						}
						gCMD=0;
						return;
					}
					break;
				case 0x0E:
					PF2_LED_ON; //yellow
					b1=I2C_write (gAddr, gCNT, outb) ;
					b2=0;
					break;				
				}				
				putByte(b1);
				putByte(b2);
				gCMD=0;
			}
			else
			{
				putByte(gFileCheckSum);
				putByte(gRxData);
				gCMD=0;
			}
			gRx1_DStep = 0;
			break;
	}
	return;
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


	// Analog Comparator initialization
	// Analog Comparator: Off
	// Analog Comparator Input Capture by Timer/Counter 1: Off
	// Analog Comparator Output: Off
	ACSR=0x80;
	SFIOR=0x00;

    //ADC initialization
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
void SW_init(void) {

	PF1_LED1_OFF;			// LED states
	PF1_LED2_OFF;
	PF2_LED_OFF;
	PWR_LED1_OFF;
	PWR_LED2_OFF;
	RUN_LED1_OFF;
	RUN_LED2_OFF;
	ERR_LED_OFF;

	PSD_OFF;                // PSD distance sensor off
}

#define PF1_BTN_PRESSED 1
#define PF2_BTN_PRESSED 3

BYTE	gBtn_val;
WORD	gBtnCnt;


//(CHK_BIT5(PORTA))
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

void ProcButton(void)
{
	int cnt;
	
	if(gBtn_val == PF1_BTN_PRESSED)
	{
		//look to see if PF2 held down
		gBtn_val = 0;
		
		printline(ver);
		printline("charge mode - testing");
		
		for (cnt=0; cnt<10; cnt++)
		{
			PWR_LED2_ON;	// RED on
			delay_ms(50);
			Get_VOLTAGE();
			DetectPower();
			printint (gVOLTAGE); printline(" mV");
			PWR_LED2_OFF;   // RED off
			delay_ms(50);
		}
				
		if(F_PS_PLUGGED)
		{	
			printline("Plugged in - charging");		
			BreakModeCmdSend();		// put servo in breakmode (power off)
			ChargeNiMH();  			//initiate battery charging	
			printline("Complete");	
		}	
		else
		{
			printline("Not plugged in");
			PWR_LED2_ON	;	
		}
	}
	
	if(gBtn_val == PF2_BTN_PRESSED)
	{
		//look to see if PF1 held down
		printline(ver);
		gBtn_val = 0;
	}
}


//------------------------------------------------------------------------------
// Send message to Sound IC
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// UART0 Receive  Routine
//------------------------------------------------------------------------------
extern volatile WORD   g10Mtimer;

int getWck(WORD timeout)
{
	g10Mtimer = timeout;
	
	while(!(UCSR0A&(1<<RXC)) ){ 	// test for received character
		if (g10Mtimer==0) return -1;
	}
	return UDR0;
}

//------------------------------------------------------------------------------
// UART0 Transmit  Routine
//------------------------------------------------------------------------------

void putWck (BYTE b)
{
	while ( (UCSR0A & DATA_REGISTER_EMPTY) == 0 ) ; //{s=!s; delay_ms(1); if (s) PWR_LED1_ON; else PWR_LED1_OFF; }
	UDR0 = b;
}

WORD Sound_Length[25]={
2268,1001,832,365,838,417,5671,5916,780,2907,552,522,1525,2494,438,402,433,461,343,472,354,461,458,442,371};

void SendToSoundIC(BYTE cmd) 
{
	BYTE	CheckSum; 
	
	if (cmd <1 || cmd>25)
	{
		putByte(0x00);
		putByte(0xff);
		return;
	}
	
	TIMSK |= 0x01;
	EIMSK |= 0x40;
		
	PWR_LED2_ON;	// RED on
	UCSR0B = (1<<RXEN)|(1<<TXEN); //enable reads for GetPos !!

	CheckSum = (29^cmd)&0x7f;
	putWck(0xFF);
	delay_ms(1);

	putWck(29);

	
	delay_ms(1);
	putWck(cmd);
	delay_ms(1);
	putWck(CheckSum);
	
	PWR_LED2_OFF; // RED off	
		
	//wait for response?
	
	PWR_LED1_ON;	// RED on
	
	WORD timeo = Sound_Length[cmd-1] + 200;
	
	int b1 = getWck(timeo);
	
	putByte(b1);
	putByte(b1);
	
	UCSR0B = (1<<RXEN)|(1<<TXEN) |(1<<RXCIE); //enable reads for GetPos !!
	PWR_LED1_OFF; // RED off	
} 

void sound_init()
{
	// low -> high PIN
	// defined in main.h
	P_BMC504_RESET(0);
	delay_ms(20);
	P_BMC504_RESET(1);
}

//------------------------------------------------------------------------------
// output routines
//------------------------------------------------------------------------------

void printstr(char *c)
{
	char ch;
	while ((ch=*c++) != 0) putch(ch);
}

void printline(char *c)
{
	printstr(c);
	putch(13);
	putch(10);
}

void printnumber(int n, int w, char pad) 
{
	char tb[10];
	char *cp=tb;
	int n1 = (n<0)?-n:n;
	while (n1>9)
	{
		*cp++ = '0' + (n1%10);
		n1 = n1/10;
		w--;
	}
	*cp++ = '0' + (n1%10);
	w--;
	if (n<0) *cp++ = '-';
	while (w-- > 0) *cp++ = pad;
	while (cp>tb)
	{
		cp--;
		putch(*cp);
	}
}

void printint(int n) 
{
	printnumber(n, 0, '\0');
}


void testI2C();
//------------------------------------------------------------------------------
// Main Routine
//------------------------------------------------------------------------------
	
int main(void) 
{
	HW_init();					// Initialise ATMega Ports
	SW_init();					// Initialise software states
			
	sei();						// enable interrupts	
	TIMSK |= 0x01;		
	
	PWR_LED1_ON; 				// Power green light on
	
	//testI2C();
		
	sound_init();
		
	Acc_init();				// initialise acceleromter
	
	//call self test

	SelfTest1();	
	ReadButton();	
	ProcButton();
	
	//otherwise DC mode!
	//default sampling ON
	
	// USART1 initialization
	UCSR1A=0x00;
	UCSR1B= (1<<RXEN)|(1<<TXEN) |(1<<RXCIE); //enable PC read/write ! (old value=0x18;		
	UCSR1C=0x06;
	UBRR1H=0x00;
	UBRR1L=BR115200;

	
	TIMSK &= 0xFE;
	EIMSK &= 0xBF;
	UCSR0B |= 0x80;
	UCSR0B &= 0xBF;
	
	sample_sound(1);
	
	gCMD=0;
	PLAY_SOUND=0;
			
	while (1) 
	{
		// do nothing
		if (PLAY_SOUND!=0)
		{
			SendToSoundIC(PLAY_SOUND);
			PLAY_SOUND=0;
		}
	} 

}