//==============================================================================
// Interface to the IR controller
//==============================================================================
extern volatile BYTE	gIRReady;				// IR message received
extern volatile BYTE	gIRData;				// Data from IR
extern volatile BYTE	gIRAddr;				// Data from IR

extern BYTE	IRState;							// state or IR receive:
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
