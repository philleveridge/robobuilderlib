//==============================================================================
//	 Compatability mode
//==============================================================================
#include <avr/io.h>
#include <stdio.h>

#include "Main.h"

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h> 


#include "Macro.h"

#include "Adc.h"
#include "math.h"
//#include "accel.h"
#include "uart.h"

#include "compatability.h"

#include <util/delay.h>

void M_Play(BYTE a)
{
	SampleMotion(a); 
}


void	sciTx1Data(BYTE x)
{
	uartSendByte(x);
}

void 	delay_ms(int x) {_delay_ms(x);}

struct TwCK_in_Motion{
	BYTE	Exist;
	BYTE	RPgain;
	BYTE	RDgain;
	BYTE	RIgain;
	BYTE	PortEn;
	BYTE	InitPos;
};

struct TMotion{
	BYTE	RIdx;
	DWORD	AIdx;
	BYTE	PF;
	WORD	NumOfScene;
	WORD	NumOfwCK;
	struct	TwCK_in_Motion  wCK[MAX_wCK];
	WORD	FileSize;
}Motion;



BYTE	F_PF;
BYTE	F_ERR_CODE; 


BYTE 	F_SCENE_PLAYING;
BYTE  	F_ACTION_PLAYING;
BYTE  	F_DIRECT_C_EN;
BYTE  	F_MOTION_STOPPED;
BYTE    F_DOWNLOAD;
BYTE  	F_PS_PLUGGED;
BYTE  	F_CHARGING;
BYTE    F_AD_CONVERTING;


BYTE    F_MIC_INPUT;
BYTE    F_PF_CHANGED;
BYTE    F_IR_RECEIVED;
BYTE    F_FIRST_M;
BYTE 	F_RSV_MOTION;
BYTE 	F_RSV_SOUND_READ;
BYTE 	F_RSV_BTN_READ;
BYTE 	F_RSV_PSD_READ;


BYTE	F_RSV_IR_READ;
BYTE	F_EEPROM_BUSY;


char	gRx1Buf[RX1_BUF_SIZE];
WORD	gRx1Step;
WORD	gRx1_DStep;
WORD	gFieldIdx;
BYTE	gFileCheckSum;
BYTE	gRxData;

WORD	gPF1BtnCnt;
WORD	gPF2BtnCnt;
WORD	gPF12BtnCnt;
BYTE	gBtn_val;


WORD    gPSplugCount;
WORD    gPSunplugCount;
WORD	gPwrLowCount;

char	gIrBuf[IR_BUFFER_SIZE];
BYTE	gIrBitIndex = 0;

signed char	gAccX;
signed char	gAccY;
signed char	gAccZ;

BYTE	gSoundMinTh;

WORD	gScIdx;

uint8_t EEMEM       eData[13];
uint8_t EEMEM 		eRCodeH[NUM_OF_REMOCON];
uint8_t EEMEM 		eRCodeM[NUM_OF_REMOCON];
uint8_t EEMEM 		eRCodeL[NUM_OF_REMOCON];
uint8_t EEMEM 		eM_OriginPose[NUM_OF_WCK_HUNO]={
/* ID
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
124,202,162, 65,108,125, 46, 88,184,142, 90, 40,125,161,210,126};

int	gPoseDelta[31];

uint8_t  EEMEM 	ePF = 1;
uint8_t  EEMEM  eNumOfM = 0;
uint8_t  EEMEM  eNumOfA = 0;
uint16_t EEMEM  eM_Addr[20];
uint16_t EEMEM  eM_FSize[20];
uint16_t EEMEM  eA_Addr[10];
uint16_t EEMEM  eA_FSize[10];

BYTE    gDownNumOfM;
BYTE    gDownNumOfA;

int Round(float num,int precision)
{
	float tempNum;
	tempNum = num;
	if(tempNum - floor(tempNum) >= 0.5)
		return (int)( ceil(tempNum)  );
	else return (int)(floor(tempNum));
}

void SendToPC(BYTE Cmd, BYTE CSize)
{
	sciTx1Data(0xFF);
	sciTx1Data(0xFF);
	sciTx1Data(0xAA);
	sciTx1Data(0x55);
	sciTx1Data(0xAA);
	sciTx1Data(0x55);
	sciTx1Data(0x37);
	sciTx1Data(0xBA);
	sciTx1Data(Cmd);
	sciTx1Data(F_PF);
	sciTx1Data(0);
	sciTx1Data(0);
	sciTx1Data(0);
	sciTx1Data(CSize);
}

void U1I_case100(void)
{
	Motion.PF = gRxData;
	gFieldIdx = 0;
	gRx1Step++;
}


void U1I_case301(BYTE LC)
{
	gFieldIdx++;
	if(gFieldIdx == 4){
		gFieldIdx = 0;
		gFileCheckSum = 0;
		if(gRxData == LC)	
			gRx1Step++;
		else{
			gRx1Step = 0;
			F_DOWNLOAD = 0;
			RUN_LED2_OFF;
		}
	}
}

void U1I_case302(void)
{
	gFileCheckSum ^= gRxData;
	if(gRxData == 1)
		gRx1Step++;
	else{
		gRx1Step = 0;
		F_DOWNLOAD = 0;
		RUN_LED2_OFF;
	}
}

void U1I_case303(void)
{
	int		i;
	if(gRxData == 1){
		SendToPC(11,16);
		gFileCheckSum = 0;
		for(i = 0; i < 16; i++){
			sciTx1Data(eM_OriginPose[i]);
			gFileCheckSum ^= eM_OriginPose[i];
		}
		sciTx1Data(gFileCheckSum);
	}
	gRx1Step = 0;
	F_DOWNLOAD = 0;
	RUN_LED2_OFF;
}


void U1I_case502(BYTE LC)
{
	gFileCheckSum ^= gRxData;
	gFieldIdx++;
	if(gFieldIdx == LC){
		gRx1Step++;
	}
}


void U1I_case603(void)
{
	int		i;
	/*

	if(gFileCheckSum == gRxData){
		F_ERR_CODE = NO_ERR;
		for(i = 0; i < 16; i++){
			if((StandardZeroPos[i]+15) < gRx1Buf[RX1_BUF_SIZE-17+i]
			 ||(StandardZeroPos[i]-15) > gRx1Buf[RX1_BUF_SIZE-17+i]){
				F_ERR_CODE = ZERO_SET_ERR;
				break;
			}
		}
		if(F_ERR_CODE == NO_ERR){
			for(i = 0; i < 16; i++)
				eM_OriginPose[i] = gRx1Buf[RX1_BUF_SIZE-17+i];

			SendToPC(14,16);
			gFileCheckSum = 0;
			for(i = 0; i < 16; i++){
				sciTx1Data(eM_OriginPose[i]);
				gFileCheckSum ^= eM_OriginPose[i];
			}
			sciTx1Data(gFileCheckSum);
		}
	}
	*/
	gRx1Step = 0;
	F_DOWNLOAD = 0;
	RUN_LED2_OFF;
}


void U1I_case703(void)
{
	WORD    lwtmp;
	BYTE	lbtmp;

	if(gFileCheckSum == gRxData){
		if(gDownNumOfM == 0){
			if(gRx1Buf[RX1_BUF_SIZE-2] == 1)
				lwtmp = 0x6000;
			else if(gRx1Buf[RX1_BUF_SIZE-2] == 2)
				lwtmp = 0x2000;
		}
		else{
			if(gRx1Buf[RX1_BUF_SIZE-2] == 1){
				lwtmp = eM_Addr[gDownNumOfM-1] + eM_FSize[gDownNumOfM-1];
				lwtmp = lwtmp + 64 - (eM_FSize[gDownNumOfM-1]%64);
				lwtmp = 0x6000 - lwtmp;
			}
			else if(gRx1Buf[RX1_BUF_SIZE-2] == 2){
				lwtmp = eA_Addr[gDownNumOfA-1] + eA_FSize[gDownNumOfA-1];
				lwtmp = lwtmp + 64 - (eA_FSize[gDownNumOfA-1]%64);
				lwtmp = 0x8000 - lwtmp;
			}
		}
		SendToPC(15,4);
		gFileCheckSum = 0;
		sciTx1Data(0x00);
		sciTx1Data(0x00);
		lbtmp = (BYTE)((lwtmp>>8) & 0xFF);
		sciTx1Data(lbtmp);
		gFileCheckSum ^= lbtmp;
		lbtmp = (BYTE)(lwtmp & 0xFF);
		sciTx1Data(lbtmp);
		gFileCheckSum ^= lbtmp;
		sciTx1Data(gFileCheckSum);
	}
	gRx1Step = 0;
	F_DOWNLOAD = 0;
	RUN_LED2_OFF;
}

//------------------------------------------------------------------------------
// event driven
//------------------------------------------------------------------------------

void process_read()
{
    WORD    i;
    
    //gRxData = UDR1;
	if(F_DIRECT_C_EN){
		//while( (UCSR0A & DATA_REGISTER_EMPTY) == 0 );
		//UDR0 = gRxData;
		
		gRxData = uartGetByte();
				
		if(gRxData == 0xff){
			gRx1_DStep = 1;
			gFileCheckSum = 0;
			return;
		}

		
		switch(gRx1_DStep){
			case 1:
				if(gRxData == 0xe0) gRx1_DStep = 2;
				else gRx1_DStep = 0;
				gFileCheckSum ^= gRxData;
				break;
			case 2:
				if(gRxData == 251) gRx1_DStep = 3;
				else gRx1_DStep = 0;
				gFileCheckSum ^= gRxData;
				break;
			case 3:
				if(gRxData == 1) gRx1_DStep = 4;
				else gRx1_DStep = 0;
				gFileCheckSum ^= gRxData;
				break;
			case 4:
				gRx1_DStep = 5;
				gFileCheckSum ^= gRxData;
				gFileCheckSum &= 0x7F;
				break;
			case 5:
				if(gRxData == gFileCheckSum){
				    TIMSK |= 0x01;
					EIMSK |= 0x40;
					UCSR0B &= 0x7F;
					UCSR0B |= 0x40;
					F_DIRECT_C_EN = 0;
				}
				gRx1_DStep = 0;
				break;
		}
		return;
	}
	//UCSR0B &= 0xBF;
	//EIMSK &= 0xBF;

   	for(i = 1; i < RX1_BUF_SIZE; i++) gRx1Buf[i-1] = gRx1Buf[i];
   	gRx1Buf[RX1_BUF_SIZE-1] = gRxData;

    if(F_DOWNLOAD == 0
     && gRx1Buf[RX1_BUF_SIZE-8] == 0xFF
     && gRx1Buf[RX1_BUF_SIZE-7] == 0xFF
     && gRx1Buf[RX1_BUF_SIZE-6] == 0xAA
     && gRx1Buf[RX1_BUF_SIZE-5] == 0x55
     && gRx1Buf[RX1_BUF_SIZE-4] == 0xAA
     && gRx1Buf[RX1_BUF_SIZE-3] == 0x55
     && gRx1Buf[RX1_BUF_SIZE-2] == 0x37
     && gRx1Buf[RX1_BUF_SIZE-1] == 0xBA){
		F_DOWNLOAD = 1;
		F_RSV_SOUND_READ = 0;
		F_RSV_BTN_READ = 0;
		RUN_LED2_ON;
		gRx1Step = 1;

		UCSR0B |= 0x40;
		EIMSK |= 0x40;
		return;
	}

	switch(gRx1Step){          	
		case 1:
    	    if(gRxData == 11){
    	        gRx1Step = 300;
    	    }
    	    else if(gRxData == 14){
    	        gRx1Step = 600;
    	    }
    	    else if(gRxData == 16){
    	        gRx1Step = 800;
    	    }
    	    else if(gRxData == 17){
    	        gRx1Step = 900;
    	    }
    	    else if(gRxData == 18){
    	        gRx1Step = 1000;
    	    }
    	    else if(gRxData == 20){
    	        gRx1Step = 1200;
    	    }
    	    else if(gRxData == 21){
    	        gRx1Step = 1300;
    	    }
    	    else if(gRxData == 22){
    	        gRx1Step = 1400;
    	    }
    	    else if(gRxData == 23){
    	        gRx1Step = 1500;
    	    }
    	    else if(gRxData == 24){
    	        gRx1Step = 1600;
    	    }   
    	    else if(gRxData == 25){
    	        gRx1Step = 1700;
    	    }
    	    else if(gRxData == 26){
    	        gRx1Step = 1800;
    	    }
    	    else if(gRxData == 31){
    	        gRx1Step = 2300;
    	    }
    	    else{
	        gRx1Step = 0;
		F_DOWNLOAD = 0;
		RUN_LED2_OFF;
		break;
	        }    	    	
    	    break;
    	case 300:
			U1I_case100();
    	    break;
    	case 301:
			U1I_case301(1);
       	 	break;
    	case 302:
			U1I_case302();
       	 	break;
    	case 303:
    		U1I_case303();
       	 	break;
    	case 600:
			U1I_case100();
    	    break;
    	case 601:
			U1I_case301(16);
       	 	break;
    	case 602:
			U1I_case502(16);
       	 	break;
    	case 603:
    		U1I_case603();
       	 	break;
    	case 800:
			U1I_case100();
    	    break;
    	case 801:
			U1I_case301(1);
       	 	break;
    	case 802:
			U1I_case302();
       	 	break;
    	case 803:
			if(gFileCheckSum == gRxData){
				SendToPC(16,1);
				gFileCheckSum = 0;
				sciTx1Data(0x01);
				gFileCheckSum ^= 0x01;
				sciTx1Data(gFileCheckSum);
				gRx1Step = 0;
				F_DOWNLOAD = 0;
				RUN_LED2_OFF;
			    TIMSK &= 0xFE;
				EIMSK &= 0xBF;
				UCSR0B |= 0x80;
				UCSR0B &= 0xBF;
				F_DIRECT_C_EN = 1;
				PF1_LED1_ON;
				PF1_LED2_OFF;
				PF2_LED_ON;
				return;
			}
			gRx1Step = 0;
			F_DOWNLOAD = 0;
			RUN_LED2_OFF;
       	 	break;
    	case 900:
			U1I_case100();
    	    break;
    	case 901:
			U1I_case301(1);
       	 	break;
    	case 902:
			U1I_case302();
       	 	break;
    	case 903:
			if(gFileCheckSum == gRxData){
				SendToPC(17,2);
				gFileCheckSum = 0;
				sciTx1Data(F_ERR_CODE);
				gFileCheckSum ^= F_ERR_CODE;
				sciTx1Data(F_PF);
				gFileCheckSum ^= F_PF;
				sciTx1Data(gFileCheckSum);
			}
			gRx1Step = 0;
			F_DOWNLOAD = 0;
			RUN_LED2_OFF;
       	 	break;
    	case 1000:
			U1I_case100();
    	    break;
    	case 1001:
			U1I_case301(1);
       	 	break;
    	case 1002:
			U1I_case302();
       	 	break;
    	case 1003:
			if(gFileCheckSum == gRxData){
				SendToPC(18,2);
				gFileCheckSum = 0;
				sciTx1Data(9);
				gFileCheckSum ^= 9;
				sciTx1Data(99);
				gFileCheckSum ^= 99;
				sciTx1Data(gFileCheckSum);
			}
			gRx1Step = 0;
			F_DOWNLOAD = 0;
			RUN_LED2_OFF;
       	 	break;
    	case 1200:
			U1I_case100();
    	    break;
    	case 1201:
			U1I_case301(1);
       	 	break;
    	case 1202:
			gFileCheckSum ^= gRxData;
			if(gRxData < 64)
				gRx1Step++;
			else{
				gRx1Step = 0;
				F_DOWNLOAD = 0;
				RUN_LED2_OFF;
			}
       	 	break;
    	case 1203:
			if(gFileCheckSum == gRxData){
				F_RSV_MOTION = 1;
				if(gRx1Buf[RX1_BUF_SIZE-2] == 0x07)	F_MOTION_STOPPED = 1;
				gRx1Step = 0;
				F_DOWNLOAD = 0;
				RUN_LED2_OFF;
				UCSR0B |= 0x40;
				EIMSK |= 0x40;
				F_IR_RECEIVED = 1;
				gIrBuf[0] = eRCodeH[0];
				gIrBuf[1] = eRCodeM[0];
				gIrBuf[2] = eRCodeL[0];
				gIrBuf[3] = gRx1Buf[RX1_BUF_SIZE-2];
				return;
			}
			gRx1Step = 0;
			F_DOWNLOAD = 0;
			RUN_LED2_OFF;
       	 	break;
    	case 1300:
			U1I_case100();
    	    break;
    	case 1301:
			U1I_case301(1);
       	 	break;
    	case 1302:
			gFileCheckSum ^= gRxData;
			if(gRxData < 26)
				gRx1Step++;
			else{
			gRx1Step = 0;
				F_DOWNLOAD = 0;
				RUN_LED2_OFF;
			}
       	 	break;
    	case 1303:
			if(gFileCheckSum == gRxData){
				//SendToSoundIC(gRx1Buf[RX1_BUF_SIZE-2]);
				//delay_ms(200 + Sound_Length[gRx1Buf[RX1_BUF_SIZE-2]-1]);
				SendToPC(21,1);
				gFileCheckSum = 0;
				sciTx1Data(gRx1Buf[RX1_BUF_SIZE-2]);
				gFileCheckSum ^= gRx1Buf[RX1_BUF_SIZE-2];
				sciTx1Data(gFileCheckSum);
			}
			gRx1Step = 0;
			F_DOWNLOAD = 0;
			RUN_LED2_OFF;
       	 	break;
    	case 1400:
			U1I_case100();
    	    break;
    	case 1401:
			U1I_case301(1);
       	 	break;
    	case 1402:
			U1I_case302();
       	 	break;
    	case 1403:
			if(gFileCheckSum == gRxData){
				F_RSV_PSD_READ = 1;
			}
			gRx1Step = 0;
			F_DOWNLOAD = 0;
			RUN_LED2_OFF;
       	 	break;
    	case 1500:
			U1I_case100();
    	    break;
    	case 1501:
			U1I_case301(2);
       	 	break;
    	case 1502:
			U1I_case502(2);
       	 	break;
    	case 1503:
			if(gFileCheckSum == gRxData){
				gSoundMinTh = gRx1Buf[RX1_BUF_SIZE-2];
				F_RSV_SOUND_READ = 1;
			}
			gRx1Step = 0;
			F_DOWNLOAD = 0;
			RUN_LED2_OFF;
       	 	break;
    	case 1600:
			U1I_case100();
    	    break;
    	case 1601:
			U1I_case301(1);
       	 	break;
    	case 1602:
			U1I_case302();
       	 	break;
    	case 1603:
			if(gFileCheckSum == gRxData){
				F_RSV_BTN_READ = 1;
			}
			gRx1Step = 0;
			F_DOWNLOAD = 0;
			RUN_LED2_OFF;
       	 	        break; 
       	 	    	 	
        case 1700:
			U1I_case100();
    	                break;
    	case 1701:
		        U1I_case301(1);
       	 	        break;
    	case 1702:
			U1I_case302();
       	 	        break;
    	case 1703:
			if(gFileCheckSum == gRxData){
				F_RSV_IR_READ = 1;
			}
			gRx1Step = 0;
			F_DOWNLOAD = 0;
			RUN_LED2_OFF;
       	 	break;
    	case 1800:
			U1I_case100();
    	    break;
    	case 1801:
			U1I_case301(1);
       	 	break;
    	case 1802:
			U1I_case302();
       	 	break;
    	case 1803:
			if(gFileCheckSum == gRxData){
				SendToPC(26,6);
				gFileCheckSum = 0;
				if(gAccX < 0){
	    			sciTx1Data(gAccX);
    				sciTx1Data(0xff);
        	    }
            	else{
	    			sciTx1Data(gAccX);
    				sciTx1Data(0);
        	    }
				gFileCheckSum ^= gAccX;
				if(gAccY < 0){
    				sciTx1Data(gAccY);
	    			sciTx1Data(0xff);
    	        }
        	    else{
    				sciTx1Data(gAccY);
	    			sciTx1Data(0);
    	        }
				gFileCheckSum ^= gAccY;
				if(gAccZ < 0){
	    			sciTx1Data(gAccZ);
    				sciTx1Data(0xff);
        	    }
            	else{
	    			sciTx1Data(gAccZ);
    				sciTx1Data(0);
        	    }
				gFileCheckSum ^= gAccZ;
				sciTx1Data(gFileCheckSum);
			}
			gRx1Step = 0;			// 다운로드 종료
			F_DOWNLOAD = 0;			// 다운로드 중 표시 해제
			RUN_LED2_OFF;			// 연결 상태 LED 표시 해제
       	 	break;
    	case 2300:
			U1I_case100();
    	    break;
    	case 2301:
			U1I_case301(1);
       	 	break;
    	case 2302:
			gFileCheckSum ^= gRxData;
			if(gRxData < 4)
				gRx1Step++;
			else{
				gRx1Step = 0;
				F_DOWNLOAD = 0;
				RUN_LED2_OFF;
			}
       	 	break;
    	case 2303:
			if(gFileCheckSum == gRxData){
				if(gRx1Buf[RX1_BUF_SIZE-2] == 1){
					gDownNumOfM = 0;
					eNumOfM = 0;
				}
				else if(gRx1Buf[RX1_BUF_SIZE-2] == 2){
					gDownNumOfA = 0;
					eNumOfA = 0;
				}
				SendToPC(31,1);
				gFileCheckSum = 0;
				sciTx1Data(gRx1Buf[RX1_BUF_SIZE-2]);
				gFileCheckSum ^= gRx1Buf[RX1_BUF_SIZE-2];
				sciTx1Data(gFileCheckSum);
			}
			gRx1Step = 0;
			F_DOWNLOAD = 0;
			RUN_LED2_OFF;
       	 	break;
	}
	UCSR0B |= 0x40;
	EIMSK |= 0x40;
}



void ProcComm(void)
{
	BYTE	lbtmp;

	if(F_RSV_PSD_READ )
	{
		SendToPC(22,2);
		gFileCheckSum = 0;
		sciTx1Data(gDistance);
		sciTx1Data(0);
		gFileCheckSum ^= gDistance;
		sciTx1Data(gFileCheckSum);
	}
		
	if(F_RSV_IR_READ && F_IR_RECEIVED)         // Added in to support IR in remote mode
	{  
	        EIMSK &= 0xBF; 
	        F_IR_RECEIVED = 0;  
	        
	        if((gIrBuf[0]==eRCodeH[0] && gIrBuf[1]==eRCodeM[0] && gIrBuf[2]==eRCodeL[0])
		 ||(gIrBuf[0]==eRCodeH[1] && gIrBuf[1]==eRCodeM[1] && gIrBuf[2]==eRCodeL[1])
		 ||(gIrBuf[0]==eRCodeH[2] && gIrBuf[1]==eRCodeM[2] && gIrBuf[2]==eRCodeL[2])
		 ||(gIrBuf[0]==eRCodeH[3] && gIrBuf[1]==eRCodeM[3] && gIrBuf[2]==eRCodeL[3])
		 ||(gIrBuf[0]==eRCodeH[4] && gIrBuf[1]==eRCodeM[4] && gIrBuf[2]==eRCodeL[4]))
		 {
	                SendToPC(25,2);
		        gFileCheckSum = 0;
		        sciTx1Data(gIrBuf[3]);
		        sciTx1Data(0);
		        gFileCheckSum ^= gIrBuf[3];
		        sciTx1Data(gFileCheckSum);
		}  	
	}
	
	if(F_RSV_SOUND_READ){
		Get_AD_MIC();
		if(gSoundMinTh <= gSoundLevel){
			SendToPC(23,2);
			gFileCheckSum = 0;
			sciTx1Data(gSoundLevel);
			sciTx1Data(0);
			gFileCheckSum ^= gSoundLevel;
			sciTx1Data(gFileCheckSum);
		}
	}
	if(F_RSV_BTN_READ){
		lbtmp = PINA & 0x03;
		if(lbtmp == 0x02){	
			delay_ms(30);
			if(lbtmp == 0x02){
				SendToPC(24,2);
				gFileCheckSum = 0;
				sciTx1Data(1);
				sciTx1Data(0);
				gFileCheckSum ^= 1;
				sciTx1Data(gFileCheckSum);
				delay_ms(200);
			}
		}
		else if(lbtmp == 0x01){
			delay_ms(30);
			if(lbtmp == 0x01){
				SendToPC(24,2);
				gFileCheckSum = 0;
				sciTx1Data(2);
				sciTx1Data(0);
				gFileCheckSum ^= 2;
				sciTx1Data(gFileCheckSum);
				delay_ms(200);
			}
		}
	}
}

//------------------------------------------------------------------------------
// IR 수신 처리
//------------------------------------------------------------------------------
void ProcIr(void)
{
    WORD    i;

	if(F_DOWNLOAD) return;
	if(F_FIRST_M && gIrBuf[3]!=BTN_C && gIrBuf[3]!=BTN_SHARP_A && F_PF!=PF2) return;
	if(F_IR_RECEIVED && !F_RSV_IR_READ){
	    EIMSK &= 0xBF;
		F_IR_RECEIVED = 0;
		if((gIrBuf[0]==eRCodeH[0] && gIrBuf[1]==eRCodeM[0] && gIrBuf[2]==eRCodeL[0])
		 ||(gIrBuf[0]==eRCodeH[1] && gIrBuf[1]==eRCodeM[1] && gIrBuf[2]==eRCodeL[1])
		 ||(gIrBuf[0]==eRCodeH[2] && gIrBuf[1]==eRCodeM[2] && gIrBuf[2]==eRCodeL[2])
		 ||(gIrBuf[0]==eRCodeH[3] && gIrBuf[1]==eRCodeM[3] && gIrBuf[2]==eRCodeL[3])
		 ||(gIrBuf[0]==eRCodeH[4] && gIrBuf[1]==eRCodeM[4] && gIrBuf[2]==eRCodeL[4])){
			switch(gIrBuf[3]){
				case BTN_A:
					M_Play(BTN_A);
					break;
				case BTN_B:
					M_Play(BTN_B);
					break;
				case BTN_LR:
					M_Play(BTN_LR);
					break;
				case BTN_U:
					M_Play(BTN_U);
					break;
				case BTN_RR:
					M_Play(BTN_RR);
					break;
				case BTN_L:
					M_Play(BTN_L);
					break;
				case BTN_R:
					M_Play(BTN_R);
					break;
				case BTN_LA:
					M_Play(BTN_LA);
					break;
				case BTN_D:
					M_Play(BTN_D);
					break;
				case BTN_RA:
					M_Play(BTN_RA);
					break;
				case BTN_C:
					F_FIRST_M = 0;
					M_Play(BTN_C);
					break;
				case BTN_1:
					break;
				case BTN_2:
					break;
				case BTN_3:
					break;
				case BTN_4:
					break;
				case BTN_5:
					break;
				case BTN_6:
					break;
				case BTN_7:
					break;
				case BTN_8:
					break;
				case BTN_9:
					break;
				case BTN_0:
					M_Play(BTN_0);
					break;
				case BTN_STAR_A:
					M_Play(BTN_STAR_A);
					break;
				case BTN_STAR_B:
					M_Play(BTN_STAR_B);
					break;
				case BTN_STAR_C:
					M_Play(BTN_STAR_C);
					break;
				case BTN_STAR_1:
					break;
				case BTN_STAR_2:
					break;
				case BTN_STAR_3:
					break;
				case BTN_STAR_4:
					break;
				case BTN_STAR_5:
					break;
				case BTN_STAR_6:
					break;
				case BTN_STAR_7:
					break;
				case BTN_STAR_8:
					break;
				case BTN_STAR_9:
					break;
				case BTN_STAR_0:
					break;
				case BTN_SHARP_1:
					break;
				case BTN_SHARP_2:
					break;
				case BTN_SHARP_3:
					break;
				case BTN_SHARP_4:
					break;
				case BTN_SHARP_5:
					break;
				case BTN_SHARP_6:
					break;
				case BTN_SHARP_7:
					break;
				case BTN_SHARP_8:
					break;
				case BTN_SHARP_9:
					break;
				case BTN_SHARP_0:
					break;
				case BTN_SHARP_A:
					if(F_PS_PLUGGED){
						wckPowerDown();
						ChargeNiMH();
					}
					else{
					}
					break;
				case BTN_SHARP_B:
					break;
				case BTN_SHARP_C:
					BasicPose(0, 50, 1000, 4);
					BasicPose(0, 1, 100, 1);
					break;
			}
		}
		if(F_RSV_MOTION){
			F_RSV_MOTION = 0;
			SendToPC(20,1);
			gFileCheckSum = 0;
			sciTx1Data(gIrBuf[3]);
			gFileCheckSum ^= gIrBuf[3];
			sciTx1Data(gFileCheckSum);
		}
		for(i=0;i<IR_BUFFER_SIZE;i++)	gIrBuf[i]=0;
	    EIMSK |= 0x40;
	}
}

//------------------------------------------------------------------------------
// 버튼 읽기
//------------------------------------------------------------------------------
void ReadButton(void)
{
	BYTE	lbtmp;

	lbtmp = PINA & 0x03;

	if(F_DOWNLOAD) return;

	if(lbtmp == 0x02){
		gPF1BtnCnt++;		gPF2BtnCnt = 0;		gPF12BtnCnt = 0;
       	if(gPF1BtnCnt > 3000){
			gBtn_val = PF1_BTN_LONG;
			gPF1BtnCnt = 0;
		}
	}
	else if(lbtmp == 0x01){
		gPF1BtnCnt = 0;		gPF2BtnCnt++;		gPF12BtnCnt = 0;
       	if(gPF2BtnCnt > 3000){
			gBtn_val = PF2_BTN_LONG;
			gPF2BtnCnt = 0;
		}
	}
	else if(lbtmp == 0x00){
		gPF1BtnCnt = 0;		gPF2BtnCnt = 0;		gPF12BtnCnt++;
       	if(gPF12BtnCnt > 2000){
           	if(F_PF_CHANGED == 0){
				gBtn_val = PF12_BTN_LONG;
	       	    gPF12BtnCnt = 0;
			}
		}
	}
	else{
		if(gPF1BtnCnt > 40 && gPF1BtnCnt < 500){
			gBtn_val = PF1_BTN_SHORT;
		}
		else if(gPF2BtnCnt > 40 && gPF2BtnCnt < 500){
			gBtn_val = PF2_BTN_SHORT;
		}
		else if(gPF12BtnCnt > 40 && gPF12BtnCnt < 500){
			gBtn_val = PF12_BTN_SHORT;
		}
		else
			gBtn_val = BTN_NOT_PRESSED;
		gPF1BtnCnt = 0;
		gPF2BtnCnt = 0;
		gPF12BtnCnt = 0;
		F_PF_CHANGED = 0;
	}
} 

//------------------------------------------------------------------------------
// Io 업데이트 처리
//------------------------------------------------------------------------------
void IoUpdate(void)
{
	if(F_DOWNLOAD) return;
	if(F_DIRECT_C_EN){
			PF1_LED1_ON;
			PF1_LED2_OFF;
			PF2_LED_ON;
			return;
	}
	switch(F_PF){
		case PF1_HUNO:
			PF1_LED1_ON;
			PF1_LED2_OFF;
			PF2_LED_OFF;
			break;
		case PF1_DINO:
			PF1_LED1_ON;
			PF1_LED2_ON;
			PF2_LED_OFF;
			break;
		case PF1_DOGY:
			PF1_LED1_OFF;
			PF1_LED2_ON;
			PF2_LED_OFF;
			break;
		case PF2_B:
			PF1_LED1_OFF;
			PF1_LED2_OFF;
			PF2_LED_ON;
			break;
		default:
			F_PF = PF2_B;
	}

	if(gVOLTAGE>M_T_OF_POWER){
		PWR_LED1_ON;
		PWR_LED2_OFF;
		gPwrLowCount = 0;
	}
	else if(gVOLTAGE>L_T_OF_POWER){
		PWR_LED1_OFF;
		PWR_LED2_ON;
		gPwrLowCount++;
		if(gPwrLowCount>5000){
			gPwrLowCount = 0;
			wckPowerDown();
		}
	}
	else{
		PWR_LED1_OFF;
		if(g10MSEC<25)			PWR_LED2_ON;
		else if(g10MSEC<50)		PWR_LED2_OFF;
		else if(g10MSEC<75)		PWR_LED2_ON;
		else if(g10MSEC<100)	PWR_LED2_OFF;
		gPwrLowCount++;
		if(gPwrLowCount>3000){
			gPwrLowCount=0;
			wckPowerDown();
		}
	}
	if(F_ERR_CODE == NO_ERR)	ERR_LED_OFF;
	else ERR_LED_ON;
}


//------------------------------------------------------------------------------
// 자체 테스트1
//------------------------------------------------------------------------------
void SelfTest1(void)
{
	WORD	i;

	if(F_DIRECT_C_EN)	return;

	//for(i=0;i<16;i++){
	//	if((StandardZeroPos[i]+15)<eM_OriginPose[i]
	//	 ||(StandardZeroPos[i]-15)>eM_OriginPose[i]){
	//		F_ERR_CODE = ZERO_DATA_ERR;
	//		return;
	//	}
	//}
	PWR_LED1_ON;	delay_ms(60);	PWR_LED1_OFF;
	PWR_LED2_ON;	delay_ms(60);	PWR_LED2_OFF;
	RUN_LED1_ON;	delay_ms(60);	RUN_LED1_OFF;
	RUN_LED2_ON;	delay_ms(60);	RUN_LED2_OFF;
	ERR_LED_ON;		delay_ms(60);	ERR_LED_OFF;

	PF2_LED_ON;		delay_ms(60);	PF2_LED_OFF;
	PF1_LED2_ON;	delay_ms(60);	PF1_LED2_OFF;
	PF1_LED1_ON;	delay_ms(60);	PF1_LED1_OFF;
}



//------------------------------------------------------------------------------
// 퇭ompatability mode with standard firmware
//------------------------------------------------------------------------------
void compatability_mode(void) {
	WORD    l10MSEC=0;

	P_BMC504_RESET(0);
	delay_ms(20);
	P_BMC504_RESET(1);

	SelfTest1();
	while(1){
		ReadButton();
		ProcButton();
		IoUpdate();
		if(g10MSEC == 0 || g10MSEC == 50)
		{
			if(g10MSEC != l10MSEC)
			{
				l10MSEC = g10MSEC;
				Get_VOLTAGE();
				DetectPower();
			}
		}
		ProcIr();
		AccGetData();
		ProcComm();
	}
}
