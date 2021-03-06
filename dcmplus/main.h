//==============================================================================
// include file for main.c
//==============================================================================

#include "avrlibtypes.h"

//#define			EXT_INT0_ENABLE		SET_BIT6(GICR)
//#define			EXT_INT0_DISABLE	CLR_BIT6(GICR)

#define	PF1_LED1_ON			CLR_BIT2(PORTA)     // BLUE
#define	PF1_LED1_OFF		SET_BIT2(PORTA)
#define	PF1_LED2_ON			CLR_BIT3(PORTA)     // GREEN
#define	PF1_LED2_OFF		SET_BIT3(PORTA)
#define	PF2_LED_ON			CLR_BIT4(PORTA)     // YELLOW
#define	PF2_LED_OFF			SET_BIT4(PORTA)
#define	RUN_LED1_ON			CLR_BIT5(PORTA)     // BLUE
#define	RUN_LED1_OFF		SET_BIT5(PORTA)
#define	RUN_LED2_ON			CLR_BIT6(PORTA)     // GREEN
#define	RUN_LED2_OFF		SET_BIT6(PORTA)
#define	ERR_LED_ON			CLR_BIT7(PORTA)     // RED
#define	ERR_LED_OFF			SET_BIT7(PORTA)
#define	PWR_LED1_ON			CLR_BIT2(PORTG)     // GREEN
#define	PWR_LED1_OFF		SET_BIT2(PORTG)
#define	PWR_LED2_ON			CLR_BIT7(PORTC)     // RED
#define	PWR_LED2_OFF		SET_BIT7(PORTC)

#define	PSD_ON		        SET_BIT5(PORTB) 	// distance sensor on
#define	PSD_OFF			    CLR_BIT5(PORTB) 	// distance sensor off

#define	CHARGE_ENABLE       SET_BIT4(PORTB) 	// enable battery charge
#define	CHARGE_DISABLE      CLR_BIT4(PORTB)		// disable battery charge

#define	U_T_OF_POWER		9500				// highest charge voltage[mV]
#define	M_T_OF_POWER		8600				// middle charge voltage[mV]
#define	L_T_OF_POWER		8100				// lowest charge voltage[mV]

#define	MAX_wCK				31					// highest possible wCK ID

#define SDATASZ 			16					// sound sample data


#define P_BMC504_RESET(A)		if(A) SET_BIT6(PORTB);else CLR_BIT6(PORTB)
#define P_PWM_SOUND_CUTOFF(A)	if(A) CLR_BIT3(DDRE);else SET_BIT3(DDRE)

