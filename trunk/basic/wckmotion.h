//==============================================================================
// wckmotion operation functions - include file for wckmotion.c
//==============================================================================


#define flash 		PROGMEM
#define HEADER     	0xff   
#define ROTATE_CCW 	3  
#define ROTATE_CW  	4  
#define TIME_OUT1  	100  // receive character timeout [msec]
#define TIME_OUT2  	250	// timeout for Set routines that change baudrate, etc.

/*------------------ init wCK bus serial communication ----------------*/  

void wckReInit     	(unsigned int ubrr);
void wckFlush      	();

/*------------------ basic wCK bus serial communication ----------------*/  

void wckSendByte   	(char data);   // Send 1 Byte to serial port  
int  wckGetByte    	(WORD timeout); // Receive 1 Byte from serial port  
char wckSetPassive 	(char ServoID);
void wckSyncPosSend	(char LastID, char SpeedLevel, char *TargetArray, char Index);  
int  wckPosRead    	(char ServoID);
WORD wckPosSend    	(char ServoID, char Torque, char Position);
void wckWriteIO		(char ServoID, char Position) ;  
char wckPowerDown	(void);
/*------------------  ----------------*/  

const BYTE basic18[18];
const BYTE basic16[16];
extern int offset[16];

enum { AccelDecel=0, Accel, Decel, Linear };

extern int PP_mtype;

BYTE readservos   	(int n);
void standup      	(int n);
void PlayPose     	(int d, int s, int tq, unsigned char data[], int n);
void PlayMotion		(BYTE Action);

/*------------------  ----------------*/  

void send_bus_str 	(char *bus_str, int n);

/*------------------  ----------------*/  

