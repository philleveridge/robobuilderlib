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
#include "Comm.h"

#include <util/delay.h>

// U_T_OF_POWER		9500 (0x251C)	- highest charge voltage[mV]
// M_T_OF_POWER		8600 (0x2198)	- middle  charge voltage[mV]
// L_T_OF_POWER		8100 (0x1FA4)	- lowest  charge voltage[mV]

extern WORD    gMSEC;				// 
extern BYTE    gSEC;				// 
extern BYTE    gMIN;				// 
extern BYTE    gHOUR;				// 

void  test_voltage(WORD volts);
int ov_flag ;

void battery_charge()
{
	WORD volts=0;

	set_break_mode();   		// Power off servos
	
	ov_flag=FALSE ;
	
	PWR_LED2_OFF;				//Power red light off  (PC7 low)

	while (gMIN < 5 && !ov_flag)  //5 mins
	{
		PWR_LED1_ON;			//   green LED  on (Port G )
		
		volts = adc_volt();

		test_voltage(volts);
		
		if (ov_flag) break;

		CHARGE_ENABLE; 			//   set charging on  (PB4 high)

		_delay_ms(40); 			//   wait 40ms

		CHARGE_DISABLE;			//   set charging off   (PB4 Low)

		_delay_ms(460); 		//   wait 460 ms
		PWR_LED1_OFF; 			//   green LED  on (Port G)
		
		volts = adc_volt();

		test_voltage(volts);

		_delay_ms(500);			//   wait 500ms		
	}
	
	PWR_LED2_ON;				//Power red light on (fast charge)
	
	int f2=0;
	CHARGE_ENABLE; 
	
	while (gMIN < 55 && !ov_flag)  // 55mins
	{
		if (f2)
			PWR_LED1_ON;			//   green LED  on (Port G )
		else
			PWR_LED1_OFF; 			//   green LED  on (Port G)

		f2 = !f2;

		_delay_ms(500); 			//   wait 1/2 s

		volts = adc_volt();

		test_voltage(volts);
	}	
	CHARGE_DISABLE;	
	rprintf("Charging complete - %dmV\r\n", volts);
}


void  test_voltage(WORD volts)
{
	static int lowCount, highCount;  

	if (ov_flag)
	{
		// if battery was full...
		if (volts > U_T_OF_POWER)
		{
			lowCount = 0;
		}
		else
		{
			lowCount += 1;
		}
		if (lowCount > 7)
		{
			ov_flag = FALSE;            // "battery not full"
			lowCount = 0;
		}
	}
	else   
	{
		// if battery was not full..
		if (volts > U_T_OF_POWER )
		{
			lowCount = 0;
			highCount += 1;
		}
		else
		{
			highCount = 0;
		}
		if (highCount > 3)
		{
			ov_flag = TRUE  ;           // "battery full"
			highCount = 0;
		}
	} 
}


