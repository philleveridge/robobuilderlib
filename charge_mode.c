//*****************************************************************************//
// File Name	: 'charge_mode.c'
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
#include "comm.h"
#include "majormodes.h"
#include "uart.h"


#include <util/delay.h>

// U_T_OF_POWER		9500 (0x251C)	- highest charge voltage[mV]
// M_T_OF_POWER		8600 (0x2198)	- middle  charge voltage[mV]
// L_T_OF_POWER		8100 (0x1FA4)	- lowest  charge voltage[mV]


void  test_voltage(WORD volts);
int ov_flag ;

void battery_charge()
{
	WORD volts=0;
	
	rprintf("Battery charging\r\n");

	set_break_mode();   		// Power off servos
	
	ov_flag=FALSE ;
	
	PWR_LED2_OFF;				//Power red light off  (PC7 low)

	while (gMIN < 5)  // trickle charge for 5 mins while battery is full
	{
		PWR_LED1_ON;			//   green LED  on (Port G )
		
		volts = adc_volt();
		test_voltage(volts);

		if (!ov_flag) break;	// 	if battery is not full, go to charge cycle
		
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
	
	while (gMIN < 55 && !ov_flag)  // fast charge 55 mins or until full
	{
		if (f2)
			PWR_LED1_ON;			//   green LED  on (Port G )
		else
			PWR_LED1_OFF; 			//   green LED  on (Port G)

		f2 = !f2;

		_delay_ms(500); 			//   wait 1/2 s

		volts = adc_volt();
		test_voltage(volts);
		if (f2) rprintf("%d mV\n", volts);
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


static void handle_serial(int cmd) {
	switch (cmd) {
	case '?':
		rprintf("Charge mode\n");
		break;
	case 'v':
		rprintf("Battery voltage: %d mV\n", adc_volt());
		break;
	case 'c':
		rprintf("Initiating charge cycle\n");
		battery_charge();
		break;
	case 27:
		rprintf("Exiting charge mode\n");
		gNextMode = kIdleMode;
		break;
	}
}

void charge_mainloop(void) {

	int cmd;

	rprintf("Charge mode\n");
	
	while (kChargeMode == gNextMode) {

		cmd = uartGetByte();
		if (cmd >= 0) handle_serial(cmd);

		// Meanwhile... continue charging the battery?
		// Not entirely sure how to integrate this with the above.
	}
}
