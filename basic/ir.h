//==============================================================================
// Interface to the IR controller
//==============================================================================

#include "avrlibtypes.h"
#include "macro.h"

extern volatile BYTE	gIRReady;				// IR message received
extern volatile BYTE	gIRData;				// Data from IR
extern volatile BYTE	gIRAddr;				// Data from IR

extern volatile BYTE	IRState;							// state or IR receive:
#define	IR_IDLE				0					// IR waiting for start
#define IR_START			1					// IR in start bit
#define IR_RECEIVE			2					// IR is catching bits
#define IR_START_SHORT		58					// IR start width minimum 
#define IR_START_LONG		82					// IR start width maximum
#define IR_BIT_SHORT		13					// IR bit width minimum
#define IR_BIT_LONG			26					// IR bit width maximum
#define IR_ZERO_ONE			19					// IR Zero/One threshold
#define IR_ADDRESS			0x2B				// Address code from IR

// get the next char from the IR receiver, or -1 if none (getchar-style)
int irGetByte(void);


//==============================================================================
//						IR ž®žðÄÁ °ü·Ã
//==============================================================================
#define NUM_OF_REMOCON		5
#define IR_BUFFER_SIZE		4
#define IR_HEADER_LT		63
#define IR_HEADER_UT		81
#define IR_LOW_BIT_LT		10
#define IR_LOW_BIT_UT		18
#define IR_HIGH_BIT_LT		19
#define IR_HIGH_BIT_UT		26

#define BTN_A		    0x01		// IR ž®žðÄÁ A
#define BTN_B		    0x02		// IR ž®žðÄÁ B
#define BTN_LR		    0x03		// IR ž®žðÄÁ ÁÂÈžÀü
#define BTN_U		    0x04		// IR ž®žðÄÁ À§·Î
#define BTN_RR		    0x05		// IR ž®žðÄÁ ¿ìÈžÀü
#define BTN_L		    0x06		// IR ž®žðÄÁ ÁÂ
#define BTN_C		    0x07		// IR ž®žðÄÁ ÁßŸÓ
#define BTN_R		    0x08		// IR ž®žðÄÁ ¿ì
#define BTN_LA		    0x09		// IR ž®žðÄÁ ÁÂ°ø°Ý
#define BTN_D		    0x0A		// IR ž®žðÄÁ ŸÆ·¡·Î
#define BTN_RA		    0x0B		// IR ž®žðÄÁ ¿ì°ø°Ý
#define BTN_1		    0x0C		// IR ž®žðÄÁ 1
#define BTN_2		    0x0D		// IR ž®žðÄÁ 2
#define BTN_3		    0x0E		// IR ž®žðÄÁ 3
#define BTN_4		    0x0F		// IR ž®žðÄÁ 4
#define BTN_5		    0x10		// IR ž®žðÄÁ 5
#define BTN_6		    0x11		// IR ž®žðÄÁ 6
#define BTN_7		    0x12		// IR ž®žðÄÁ 7
#define BTN_8		    0x13		// IR ž®žðÄÁ 8
#define BTN_9		    0x14		// IR ž®žðÄÁ 9
#define BTN_0		    0x15		// IR ž®žðÄÁ 0

#define BTN_STAR_A		0x16		// IR ž®žðÄÁ *+A
#define BTN_STAR_B		0x17		// IR ž®žðÄÁ *+B
#define BTN_STAR_LR		0x18		// IR ž®žðÄÁ *+ÁÂÈžÀü
#define BTN_STAR_U		0x19		// IR ž®žðÄÁ *+À§·Î
#define BTN_STAR_RR		0x1A		// IR ž®žðÄÁ *+¿ìÈžÀü
#define BTN_STAR_L		0x1B		// IR ž®žðÄÁ *+ÁÂ
#define BTN_STAR_C		0x1C		// IR ž®žðÄÁ *+ÁßŸÓ
#define BTN_STAR_R		0x1D		// IR ž®žðÄÁ *+¿ì
#define BTN_STAR_LA		0x1E		// IR ž®žðÄÁ *+ÁÂ°ø°Ý
#define BTN_STAR_D		0x1F		// IR ž®žðÄÁ *+ŸÆ·¡·Î
#define BTN_STAR_RA		0x20		// IR ž®žðÄÁ *+¿ì°ø°Ý
#define BTN_STAR_1		0x21		// IR ž®žðÄÁ *+1
#define BTN_STAR_2		0x22		// IR ž®žðÄÁ *+2
#define BTN_STAR_3		0x23		// IR ž®žðÄÁ *+3
#define BTN_STAR_4		0x24		// IR ž®žðÄÁ *+4
#define BTN_STAR_5		0x25		// IR ž®žðÄÁ *+5
#define BTN_STAR_6		0x26		// IR ž®žðÄÁ *+6
#define BTN_STAR_7		0x27		// IR ž®žðÄÁ *+7
#define BTN_STAR_8		0x28		// IR ž®žðÄÁ *+8
#define BTN_STAR_9		0x29		// IR ž®žðÄÁ *+9
#define BTN_STAR_0		0x2A		// IR ž®žðÄÁ *+0

#define BTN_SHARP_A		0x2B		// IR ž®žðÄÁ #+A
#define BTN_SHARP_B		0x2C		// IR ž®žðÄÁ #+B
#define BTN_SHARP_LR    0x2D		// IR ž®žðÄÁ #+ÁÂÈžÀü
#define BTN_SHARP_U		0x2E		// IR ž®žðÄÁ #+À§·Î
#define BTN_SHARP_RR	0x2F		// IR ž®žðÄÁ #+¿ìÈžÀü
#define BTN_SHARP_L		0x30		// IR ž®žðÄÁ #+ÁÂ
#define BTN_SHARP_C		0x31		// IR ž®žðÄÁ #+ÁßŸÓ
#define BTN_SHARP_R		0x32		// IR ž®žðÄÁ #+¿ì
#define BTN_SHARP_LA	0x33		// IR ž®žðÄÁ #+ÁÂ°ø°Ý
#define BTN_SHARP_D		0x34		// IR ž®žðÄÁ #+ŸÆ·¡·Î
#define BTN_SHARP_RA	0x35		// IR ž®žðÄÁ #+¿ì°ø°Ý
#define BTN_SHARP_1		0x36		// IR ž®žðÄÁ #+1
#define BTN_SHARP_2		0x37		// IR ž®žðÄÁ #+2
#define BTN_SHARP_3		0x38		// IR ž®žðÄÁ #+3
#define BTN_SHARP_4		0x39		// IR ž®žðÄÁ #+4
#define BTN_SHARP_5		0x3A		// IR ž®žðÄÁ #+5
#define BTN_SHARP_6		0x3B		// IR ž®žðÄÁ #+6
#define BTN_SHARP_7		0x3C		// IR ž®žðÄÁ #+7
#define BTN_SHARP_8		0x3D		// IR ž®žðÄÁ #+8
#define BTN_SHARP_9		0x3E		// IR ž®žðÄÁ #+9
#define BTN_SHARP_0		0x3F		// IR ž®žðÄÁ #+0
