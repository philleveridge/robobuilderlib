//*****************************************************************************//
// File Name	: 'adc.c'
// Title		: ADC single conversion witn interrupt after conversion
//
//*****************************************************************************

#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "global.h"
#include "rprintf.h"
#include "macro.h"
#include "main.h"
#include "adc.h"

#include <util/delay.h>

signed char	gAD_val;
BYTE	gAD_Ch_Index;

volatile BYTE     F_AD_CONVERTING=0;

volatile BYTE	gPSD_val;
volatile BYTE	gMIC_val;

volatile WORD	gVOLTAGE;
volatile BYTE	gDistance;
volatile BYTE	gSoundLevel;

volatile BYTE 	sData[SDATASZ];
int 	sDcnt;

void ADC_set(BYTE);

volatile BYTE   MIC_SAMPLING=0;
volatile BYTE   MIC_LEVEL=0;
volatile WORD   MIC_DLY  =0;
volatile BYTE   MIC_STOP =0;
volatile BYTE   MIC_RATE =4;
volatile BYTE   MIC_NOS  =SDATASZ;

//i.e. 10,33,50,66,90
void  blights(int n, int *vals) 
{
		RUN_LED1_OFF;
		RUN_LED2_OFF;
		ERR_LED_OFF;
		PWR_LED1_OFF;
		PWR_LED2_OFF;	

		if (n > vals[0])   ERR_LED_ON;
		if (n > vals[1])   RUN_LED1_ON;		
		if (n > vals[2])   RUN_LED2_ON;		
		if (n > vals[3])   PWR_LED1_ON;		
		if (n > vals[4])   PWR_LED2_ON;		
}

//4, 8, 16,32,64
void  lights(int n) // power bar meter!
{
	int range[5] = {4,8,16,32,64};
	blights(n, range);
}

void sample_sound(int status)
{
	if (status)
	{
		// 	set timer interupt on
		sDcnt=0;

		TIMSK |= 0x01;
		EIMSK |= 0x40;
		RUN_LED1_ON;		
		MIC_SAMPLING=1;
	}
	else
	{
		// 	set timer interupt off	
		TIMSK &= 0xFE;
		EIMSK &= 0xBF;
		lights(0);
		MIC_SAMPLING=0;
	}
}


/********************************************************************************/
extern volatile WORD	gtick;

ISR(ADC_vect)
{
	WORD i;
	gAD_val=(signed char)ADCH;
	switch(gAD_Ch_Index)
	{
		case PSD_CH:
    	    		gPSD_val = (BYTE)gAD_val;
			break; 
		case VOLTAGE_CH:
			i = (BYTE)gAD_val;
			gVOLTAGE = i*57;
			break; 
		case MIC_CH:
			if((BYTE)gAD_val < 230)
				gMIC_val = (BYTE)gAD_val;
			else
				gMIC_val = 0;
				
			lights(gMIC_val);
			
			if (sDcnt==0 && MIC_LEVEL>0 && gMIC_val<=MIC_LEVEL)
			{
				MIC_DLY--;
				if (MIC_DLY==0)
				{
					MIC_STOP=0;
					MIC_SAMPLING=0;
				}
				break; // not triggered
			}
			
			if (MIC_LEVEL>0)
				sData[sDcnt] = gMIC_val; // triggered mode
			else
				sData[sDcnt] = (sData[sDcnt] + gMIC_val)/2; //free run mode
				
			if ((sDcnt==(MIC_NOS-1)) && MIC_STOP==1) //reached end
			{
				MIC_STOP=0;
				MIC_SAMPLING=0;
			}
			sDcnt = (sDcnt + 1) % MIC_NOS;  // inc sample
			break; 
	}  
	F_AD_CONVERTING = 0;    
}

void ADC_set(BYTE mode)
{                                    
	ADMUX=0x20 | gAD_Ch_Index;
	ADCSRA=mode;     
}	

void PSD_on(void)
{
	PSD_ON;
   	_delay_ms(50);
}

void PSD_off(void)
{
	PSD_OFF;
}

void Get_AD_PSD(void)
{
	float	tmp = 0;
	float	dist;
		
	if (CHK_BIT5(PORTB)==0) PSD_on();
	
	EIMSK &= 0xBF;

	gAD_Ch_Index = PSD_CH;
	
   	ADC_set(ADC_MODE_SINGLE);
	
    while (bit_is_set(ADCSRA, ADSC));
	            
   	tmp = tmp + gPSD_val;
	//EIMSK |= 0x40;

	dist = 1117.2 / (tmp - 6.89);
	if(dist < 0) dist = 50;
	else if(dist < 10) dist = 10;
	else if(dist > 50) dist = 50;
	gDistance = (BYTE)dist;
}


//------------------------------------------------------------------------------
// MIC ��ȣ A/D
//------------------------------------------------------------------------------
void Get_AD_MIC(void)
{
	WORD	i;
	float	tmp = 0;
	
	gAD_Ch_Index = MIC_CH;
	for(i = 0; i < 50; i++)
	{	
    		F_AD_CONVERTING = 1;
	   	ADC_set(ADC_MODE_SINGLE);
    		while(F_AD_CONVERTING);            
    		tmp = tmp + gMIC_val;
    	}
    	tmp = tmp / 10;
	gSoundLevel = (BYTE)tmp;
}


//------------------------------------------------------------------------------
// ��� ��� A/D
//------------------------------------------------------------------------------
void Get_VOLTAGE(void)
{
        BYTE t=MIC_SAMPLING;			
	MIC_SAMPLING=0; // turn off mic sampling

	gAD_Ch_Index = VOLTAGE_CH;
	ADC_set(ADC_MODE_SINGLE);
	while (bit_is_set(ADCSRA, ADSC));	    // wait until value ready

	MIC_SAMPLING=t;	
}
