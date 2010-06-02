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


//==============================================================================
//						UART constants
//==============================================================================
#define TX0_BUF_SIZE    186     // UART0 TX buffer size (equals 31*6 = 186)
#define RX0_BUF_SIZE    8		// UART0 RX buffer size
#define RX1_BUF_SIZE    20      // UART1 RX buffer size

//enumerated parameters (event trigger levels)
// (we should probably move these over into the experimental module)
#define TIMR  0
#define TLT   1
#define TORQ  2
#define PSDL  3
#define PSDL2 4
#define PSDL3 5
#define MICL  6
#define MICL2 7
#define MICL3 8
#define VOLT  9


// timer variables----------------------------------------------------------------
extern volatile WORD    g10MSEC;
extern volatile WORD    gMSEC;
extern volatile BYTE    gSEC;
extern volatile BYTE    gMIN;
extern volatile BYTE    gHOUR;
extern volatile WORD	gTicks;
extern volatile WORD	gSEC_DCOUNT;
extern volatile WORD	gMIN_DCOUNT;

// UART variables-----------------------------------------------------------------
extern volatile BYTE	gTx0Buf[TX0_BUF_SIZE];		// UART0 transmit buffer
extern volatile BYTE	gTx0Cnt;					// UART0 transmit length
extern volatile BYTE	gRx0Cnt;					// UART0 receive length
extern volatile BYTE	gTx0BufIdx;					// UART0 transmit pointer
extern volatile BYTE	gRx0Buf[RX0_BUF_SIZE];		// UART0 receive buffer

#ifndef HUNO_MODEL
#define HUNO_MODEL
enum {HUNO_BASIC, HUNO_ADVANCED, HUNO_OTHER};
#endif
