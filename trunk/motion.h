#define	RXB8	1
#define	TXB8	0
#define	UPE		2
#define	OVR		3
#define	FE		4
#define	UDRE	5
#define	RXC		7

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)

// Frequency = 14.7456MHz 
// Baud rates based on CPU Frequency 14.7456 MHz 
#define BR9600		95 
#define BR19200		47 
#define BR38400		23 
#define BR57600		15
#define BR115200	7 
#define BR460800	1 

#define HEADER			0xFF 
#define WHEEL_ACT_CCW 	3 
#define WHEEL_ACT_CW	4 

#define START			0
#define STOP			1
#define PAUSE			2
#define RESUME    		3

#define RX_T_OUT    	100  // receive character timeout[msec]

#define flash PROGMEM


void sciTx0Data(BYTE td);
BYTE sciRx0Ready(void);
void SendTGain(void);
void SendExPortD(void);
void SampleMotion(int);
void BasicPose();
void set_break_mode();
void LoadMotionFromBuffer(unsigned char *motionBuf);
void PlaySceneFromBuffer(unsigned char *motionBuf, WORD sceneIndex);
void ProcessFrames();

// software states----------------------------------------------------------------------
extern volatile BYTE F_PLAYING;				// state: playing from Flash
extern volatile BYTE F_NEXTFRAME;			// trigger to start the next frame
extern volatile WORD gSceneIndex;			// which scene we're currently playing
