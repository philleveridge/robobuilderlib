//*****************************************************************************//
// File Name	: 'battery.c'
// Title		: Battery charging routines
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
#include "adc.h"
#include "main.h"

#include <util/delay.h>

// U_T_OF_POWER		9500 (0x251C)	- highest charge voltage[mV]
// M_T_OF_POWER		8600 (0x2198)	- middle  charge voltage[mV]
// L_T_OF_POWER		8100 (0x1FA4)	- lowest  charge voltage[mV]

extern WORD    gMSEC;				// 
extern BYTE    gSEC;				// 
extern BYTE    gMIN;				// 
extern BYTE    gHOUR;				// 


void batt_charge()
{
//	The basic pseudo code is some thing like this
//
	PWR_LED2_OFF;				//Power red light off  (PC7 low)
	
	int not_charged=1;

	while (not_charged)
	{
		WORD v=adc_volt(); 		//   read voltage  (ADC1)
		rprintf("%d:%d:%d-%d ",gHOUR,gMIN,gSEC,gMSEC);
		rprintf("Volts = %d - ", v);

		if (v > (WORD)U_T_OF_POWER) 
			not_charged=0;
		else
		{
			PWR_LED1_ON;			//   green LED  on (Port G )

			//CHARGE_ENABLE; 		//   set charging on  (PB4 high)

			_delay_ms(40); 			//   wait 40ms

			//CHARGE_DISABLE;		//   set charging off   (PB4 Low)

			_delay_ms(460); 		//   wait 460 ms
			PWR_LED1_OFF; 			//   green LED  on (Port G)

			v=adc_volt(); 			//   read voltage  (ADC1)
			rprintf(" %d\r\n", v);

			if (v > (WORD)U_T_OF_POWER) not_charged=0;

			_delay_ms(500);			//   wait 500ms
		}
	}

	PWR_LED2_ON; 				// Power red light on ( PC7 High ) 
	rprintf("Charging complete\r\n");
}

