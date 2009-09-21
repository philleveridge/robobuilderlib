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
#include "motion.h"
#include "majormodes.h"
#include "uart.h"


#include <util/delay.h>


void  test_voltage(WORD volts);
int ov_flag ;

extern BYTE F_DOWNLOAD;

BYTE F_PS_PLUGGED;
BYTE F_CHARGING;

WORD    gPSplugCount;
WORD    gPSunplugCount;
WORD	gPwrLowCount;

extern volatile WORD    gMSEC;

//------------------------------------------------------------------------------
// 전원 검사
//------------------------------------------------------------------------------
void DetectPower(void)
{
	if(F_DOWNLOAD) return;
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
// NiMH 배터리 충전
//-----------------------------------------------------------------------------
void ChargeNiMH(void)
{
	rprintf("Battery charging\r\n");

	F_CHARGING = 1;
	gMIN_DCOUNT = 5;
	while(gMIN_DCOUNT){
		PWR_LED2_OFF;
		PWR_LED1_ON;
		Get_VOLTAGE();	DetectPower();
		if(F_PS_PLUGGED == 0) break;
		CHARGE_ENABLE;
		_delay_ms(40);
		CHARGE_DISABLE;
		_delay_ms(500-40);
		PWR_LED1_OFF;
		Get_VOLTAGE();	DetectPower();
		if(F_PS_PLUGGED == 0) break;
		_delay_ms(500);
	}
	gMIN_DCOUNT = 85;
	while(gMIN_DCOUNT){
		PWR_LED2_OFF;
		if(gMSEC > 500)	PWR_LED1_ON;
		else			PWR_LED1_OFF;
		if(gMSEC == 0 || gMSEC == 500){
			Get_VOLTAGE();
			DetectPower();
		}
		if(F_PS_PLUGGED == 0) break;
		CHARGE_ENABLE;
	}
	CHARGE_DISABLE;
	F_CHARGING = 0;
	rprintf("Charging complete - %dmV\r\n", gVOLTAGE);
}


/********************************************************************************************/


static void handle_serial(int cmd) {
	switch (cmd) {
	case '?':
		rprintf("Charge mode\n");
		break;
	case 'v':
		Get_VOLTAGE();
		DetectPower();
		rprintf("Voltage = %dmv (%d)\n", gVOLTAGE, F_PS_PLUGGED);	
		break;
	case 'c':
		rprintf("Initiating charge cycle\n");
		ChargeNiMH();
		break;
	case 27:
		rprintf("Exiting charge mode\n");
		gNextMode = kIdleMode;
		break;
	}
}

void charge_mainloop(void) 
{
	WORD    lMSEC=0;
	int 	cmd;

	rprintf("Charge mode\n");
	
	while (kChargeMode == gNextMode) {

		cmd = uartGetByte();
		if (cmd >= 0) handle_serial(cmd);
		
		if(gMSEC == 0 || gMSEC == 50){
			if(gMSEC != lMSEC){
				lMSEC = gMSEC;
				Get_VOLTAGE();
				DetectPower();
				rprintf("Voltage = %d (%d)\n", gVOLTAGE, F_PS_PLUGGED);	
			}
		}

		// Meanwhile... continue charging the battery?
		// Not entirely sure how to integrate this with the above.
	}
}



