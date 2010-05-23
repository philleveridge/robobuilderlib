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
#include "Macro.h"
#include "main.h"
#include "adc.h"

#include <util/delay.h>

signed char	gAD_val;
BYTE	gAD_Ch_Index;

volatile BYTE   F_AD_CONVERTING=0;

volatile BYTE	gPSD_val;
volatile BYTE	gMIC_val;

volatile WORD	gVOLTAGE;
volatile BYTE	gDistance;
volatile BYTE	gSoundLevel;

BYTE 	sData[SDATASZ];
int 	sDcnt;

void sample_sound(int status)
{
	if (status)
	{
		// 	set timer interupt on
		sDcnt=0;
	}
	else
	{
		// 	set timer interupt off
	}
}


WORD adc_volt()
{
/*
	adc_start_conversion(1, 0xDC);	
	while (bit_is_set(ADCSRA, ADSC));	    // wait until value ready
	return volts;
	*/
	
	Get_VOLTAGE();
	return gVOLTAGE;
	
}

BYTE adc_mic()
{
	/*
	mic_vol=0;
	int i;
	for (i=0; i<50; i++)
	{
		adc_start_conversion(15, 0xDC);	
		while (bit_is_set(ADCSRA, ADSC));	// wait until value ready
	
		mic_vol = mic_vol + mic_value;	
	}
	return mic_vol;
	*/
	Get_AD_MIC();
	return gSoundLevel;
	
}


/********************************************************************************/

/********************************************************************************/


ISR(ADC_vect)
{
	WORD i;
	gAD_val=(signed char)ADCH;
	switch(gAD_Ch_Index){
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
   	F_AD_CONVERTING = 1;
   	ADC_set(ADC_MODE_SINGLE);
	
   	//while(F_AD_CONVERTING);    
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
// MIC 신호 A/D
//------------------------------------------------------------------------------
void Get_AD_MIC(void)
{
	WORD	i;
	float	tmp = 0;
	
	gAD_Ch_Index = MIC_CH;
	for(i = 0; i < 50; i++){
	
    	F_AD_CONVERTING = 1;
	   	ADC_set(ADC_MODE_SINGLE);
    	while(F_AD_CONVERTING);            
    	tmp = tmp + gMIC_val;
    }
    tmp = tmp / 10;
	gSoundLevel = (BYTE)tmp;
}


//------------------------------------------------------------------------------
// 전원 전압 A/D
//------------------------------------------------------------------------------
void Get_VOLTAGE(void)
{
	gAD_Ch_Index = VOLTAGE_CH;
	F_AD_CONVERTING = 1;
   	ADC_set(ADC_MODE_SINGLE);
	while (bit_is_set(ADCSRA, ADSC));	    // wait until value ready
	//while(F_AD_CONVERTING);
}