#include <avr/io.h>
#include <avr/interrupt.h>

#include "main.h"
#include "macro.h"
#include "adc.h"

extern volatile BYTE   MIC_SAMPLING;
	
BYTE 	F_PS_PLUGGED;
BYTE 	F_CHARGING;

volatile WORD   g10Mtimer;
volatile WORD   g10MSEC;
volatile BYTE   gSEC;
volatile BYTE   gMIN;
volatile BYTE   gHOUR;
volatile WORD   gticks;
volatile WORD	gSEC_DCOUNT;
volatile WORD	gMIN_DCOUNT;

volatile WORD   gPSplugCount;
volatile WORD   gPSunplugCount;
volatile WORD	gPwrLowCount;

 //main.c
extern void putByte();			

//femto.c
extern void printline(char *c);  
extern void printint(int);  	
extern void printstr(char*);	
extern void printnumber(int n, int w, char pad)  ;

//adc.c
extern BYTE	gAD_Ch_Index;
void ADC_set(BYTE );

//------------------------------------------------------------------------------
// set wck modules to btreak mode
//------------------------------------------------------------------------------
#define HEADER			0xFF 
void BreakModeCmdSend(void)
{
	BYTE	Data1, Data2;
	BYTE	CheckSum; 

	Data1 = (6<<5) | 31;
	Data2 = 0x20;
	CheckSum = (Data1^Data2)&0x7f;

	putByte(HEADER);
	putByte(Data1);
	putByte(Data2);
	putByte(CheckSum);
} 

//------------------------------------------------------------------------------
// Timer
//------------------------------------------------------------------------------

void heart()
{
	// the beating heart ...
	// 1000 ms 
	// 0-200 ms PF1 LED ON  200-400 ms PF2 LED on
	// else off
	if (g10MSEC<200)
	{
		PF1_LED1_ON;    
		PF1_LED2_OFF;
		PF2_LED_OFF;
	}
	else if (g10MSEC>200 && g10MSEC<400)
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

void delay_ms(int ms)
{
	g10Mtimer=ms;
	while (g10Mtimer>0)
	{
		//g10Mtime is decremented evry 1ms by interupt.
	}
}

ISR(TIMER0_OVF_vect)
{
	//TCNT0 = 111;
	TCNT0 = 25;
	
	gticks++;

	if (g10Mtimer>0) g10Mtimer--; //count down timer
	
	//
	if (MIC_SAMPLING && g10MSEC%4==0)
	{
		//every 4ms
		gAD_Ch_Index = MIC_CH;		
		ADC_set(ADC_MODE_SINGLE);
	}

	if (MIC_SAMPLING && g10MSEC%250==0)
	{
		//Get_AD_PSD();
	}

	heart();
	
	if(++g10MSEC > 999){
        g10MSEC = 0;
        if(gSEC_DCOUNT > 0)	gSEC_DCOUNT--;
        if(++gSEC > 59){
            gSEC = 0;
	        if(gMIN_DCOUNT > 0)	gMIN_DCOUNT--;
            if(++gMIN > 59){
                gMIN = 0;
                if(++gHOUR > 23)
                    gHOUR = 0;
            }
		}
    }
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
	while(gMIN_DCOUNT)
	{
	    if (gSEC%5==0)
		{
			printnumber(gMIN, 2,'0');
			printstr(":");
			printnumber(gSEC,2,'0');
			printline (" short 40ms charge pulses");
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
		if (gSEC%5==0)
		{
			printnumber(gMIN,2,'0');
			printstr(":");
			printnumber(gSEC, 2, '0');
			printline (" full charge power");
		}
		PWR_LED2_OFF;
		if(g10MSEC > 500)	
			PWR_LED1_ON;
		else			
			PWR_LED1_OFF;
		if(g10MSEC == 0 || g10MSEC == 500){
			Get_VOLTAGE();
			DetectPower();
		}
		if(F_PS_PLUGGED == 0) break;
		CHARGE_ENABLE;
	}
	CHARGE_DISABLE;
	F_CHARGING = 0;
	printline ("Done");
}


//------------------------------------------------------------------------------
// Simple self tests
//------------------------------------------------------------------------------

void test()
{
	if(gVOLTAGE>M_T_OF_POWER){
		PWR_LED1_ON;
		PWR_LED2_OFF;
		gPwrLowCount = 0;
	}
	else if(gVOLTAGE>L_T_OF_POWER){
		PWR_LED1_OFF;
		PWR_LED2_ON;
		gPwrLowCount++;
		if(gPwrLowCount>5000){
			gPwrLowCount = 0;
			BreakModeCmdSend();
		}
	}
	else{
		PWR_LED1_OFF;
		if(g10MSEC<25)			PWR_LED2_ON;
		else if(g10MSEC<50)		PWR_LED2_OFF;
		else if(g10MSEC<75)		PWR_LED2_ON;
		else if(g10MSEC<100)	PWR_LED2_OFF;
		gPwrLowCount++;
		if(gPwrLowCount>3000){
			gPwrLowCount=0;
			BreakModeCmdSend();
		}
	}
}

void SelfTest1(void)
{
	PWR_LED1_ON;	delay_ms(60);	PWR_LED1_OFF;
	PWR_LED2_ON;	delay_ms(60);	PWR_LED2_OFF;
	RUN_LED1_ON;	delay_ms(60);	RUN_LED1_OFF;
	RUN_LED2_ON;	delay_ms(60);	RUN_LED2_OFF;
	ERR_LED_ON;		delay_ms(60);	ERR_LED_OFF;

	PF2_LED_ON;		delay_ms(60);	PF2_LED_OFF;
	PF1_LED2_ON;	delay_ms(60);	PF1_LED2_OFF;
	PF1_LED1_ON;	delay_ms(60);	PF1_LED1_OFF;
}
