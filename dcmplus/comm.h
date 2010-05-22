#define	TXB8	0
#define	RXB8	1
#define	UPE		2
#define	OVR		3
#define	FE		4
#define	UDRE	5
#define	RXC		7

#define FRAMING_ERROR		(1<<FE)
#define PARITY_ERROR		(1<<UPE)
#define DATA_OVERRUN		(1<<OVR)
#define DATA_REGISTER_EMPTY	(1<<UDRE)
#define RX_COMPLETE			(1<<RXC)

#define	TX0_ON			SET_BIT3(UCSR0B)
#define	TX0_OFF			CLR_BIT3(UCSR0B)
#define	RX0_ON			SET_BIT4(UCSR0B)
#define	RX0_OFF			CLR_BIT4(UCSR0B)
#define	TX0_INT_ON		SET_BIT6(UCSR0B)
#define	TX0_INT_OFF		CLR_BIT6(UCSR0B)
#define	RX0_INT_ON		SET_BIT7(UCSR0B)
#define	RX0_INT_OFF		CLR_BIT7(UCSR0B)

#define	TX1_ON			SET_BIT3(UCSR1B)
#define	TX1_OFF			CLR_BIT3(UCSR1B)
#define	RX1_ON			SET_BIT4(UCSR1B)
#define	RX1_OFF			CLR_BIT4(UCSR1B)
#define	TX1_INT_ON		SET_BIT6(UCSR1B)
#define	TX1_INT_OFF		CLR_BIT6(UCSR1B)
#define	RX1_INT_ON		SET_BIT7(UCSR1B)
#define	RX1_INT_OFF		CLR_BIT7(UCSR1B)

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

#define RX_T_OUT    	20

void sciTx0Data(BYTE td);
void sciTx1Data(BYTE td);
BYTE sciRx0Ready(void);
BYTE sciRx1Ready(void);
void SendOperCmd(BYTE Data1,BYTE Data2);
void SendSetCmd(BYTE ID, BYTE Data1, BYTE Data2, BYTE Data3);
void PosSend(BYTE ID, BYTE SpeedLevel, BYTE Position);
void PassiveModeCmdSend(BYTE ID);
void BreakModeCmdSend(void);
void BoundSetCmdSend(BYTE ID, BYTE B_U, BYTE B_L);
void SyncPosSendTune(void);
void SyncPosSend(void);
WORD PosRead(BYTE ID);
void SendToSoundIC(BYTE cmd);
void SendToPC(BYTE Cmd, BYTE CSize);
void MotionTweenFlash(BYTE GapMax);
void GetMotionFromFlash(void);
void SendTGain(void);
void SendExPortD(void);
void GetSceneFromFlash(void);
void CalcFrameInterval(void);
void CalcUnitMove(void);
void MakeFrame(void);
void SendFrame(void);
void GetPose(void);
void BasicPose(BYTE PF, WORD NOF, WORD RT, BYTE TQ);
void M_Play(BYTE BtnCode);

