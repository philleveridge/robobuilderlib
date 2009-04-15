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
#include "Macro.h"
#include "main.h"

#include <util/delay.h>


void adc_init(void);
void adc_start_conversion(BYTE, BYTE);

//current channel
volatile BYTE ch;
volatile BYTE psd_value;
volatile BYTE mic_value;
volatile WORD volts;
volatile WORD mic_vol;


//*****************************************************************************//
//  ADC module initialization  
//
//*****************************************************************************
void adc_init(void)
{
	//AVC with optional capacitor
	ADMUX|=(0<<REFS1)|(1<<REFS0);
	ADCSRA = 0;
	
	//enable ADC with dummy conversion
	//set sleep mode for ADC noise reduction conversion
	//set_sleep_mode(SLEEP_MODE_ADC);
}

//*****************************************************************************
//
//  ADC single conversion routine  
//
//*****************************************************************************
void adc_start_conversion(BYTE channel, BYTE x)
{
	//remember current ADC channel;
	ch=channel;
	
	//set ADC channel
	ADMUX = channel | 0x20;
	
	//Start conversion with Interrupt after conversion
	ADCSRA = x;
} 


//*****************************************************************************
//
//  ADC conversion complete service routine  
//
//  ch = 0, PSD
//  ch = 1, Voltage
//  ch =15, Mic
//
//*****************************************************************************
ISR(ADC_vect)
{	
	switch (ch)
	{
		case 0:
			psd_value = ADCH;   	
			break;
		case 1:
			volts = ADCH*57;	
			break;
		case 15:
			mic_value = ADCH;  
			if (mic_value>230)
				mic_value=0;
			break;
	}
} 


//*****************************************************************************
//
//  delay 1s 
//
//*****************************************************************************

void delay_1s(void)
{
	uint8_t i;
	for(i=0;i<100;i++)
	{
		_delay_ms(10);
	}
}


//*****************************************************************************
//
//  run analog digital converter, timer.  
//
// ADCSRA = BV(ADEN) | BV(ADSC) | BV(ACIF) | BV(ACIE) | BV(ADSPS2) ; 			 //0xDC  
// ADCSRA = BV(ADEN) | BV(ADSC) | BV(ADFR) | BV(ACIE) | BV(ADSPS2) | BV(ADSP0) ; //oxED
//
//*****************************************************************************

BYTE adc_psd()
{
	PSD_ON;
	_delay_ms(50);	
	adc_start_conversion(0, 0xDC);							
	while (bit_is_set(ADCSRA, ADSC));	    // wait until value ready
	PSD_OFF;	
	return psd_value;
}

WORD adc_volt()
{
	adc_start_conversion(1, 0xDC);	
	while (bit_is_set(ADCSRA, ADSC));	    // wait until value ready
	return volts;
}

BYTE adc_mic()
{
	mic_vol=0;
	int i;
	for (i=0; i<50; i++)
	{
		adc_start_conversion(15, 0xDC);	
		while (bit_is_set(ADCSRA, ADSC));	// wait until value ready
	
		mic_vol = mic_vol + mic_value;	
	}
	return mic_vol;
}

int adc_test(BYTE debug){	
	adc_init();	
	delay_1s();	

	//read PSD	
	if (debug) rprintf("PSD=");
	adc_psd();
	if (debug) rprintf("%x", psd_value);
	
	//read Voltage	
	_delay_ms(50);	
	if (debug) rprintf(" VOLT=");
	adc_volt();
	if (debug) rprintf("%dmV", volts);

	//read MIC	
	_delay_ms(50);	
	if (debug) rprintf(" MIC=");
	adc_mic();
	if (debug) rprintf("%d\r\n", mic_vol);		

	return 0;
}