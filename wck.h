// wck.h

#ifndef WCK_H
#define WCK_H

#include "main.h"

/*------------------ basic wCK bus serial communication ----------------*/  
void wckSendByte(char data);   // Send 1 Byte to serial port  
char wckGetByte(WORD timeout); // Receive 1 Byte from serial port  

/*------------------ other Basic Functions relating to wCK module--------------------------*/  
void wckSendOperCommand(char Data1, char Data2);  
void wckSendSetCommand(char Data1, char Data2, char Data3, char Data4);  
WORD wckPosSend(char ServoID, char SpeedLevel, char Position);  
char wckPosRead(char ServoID);
WORD wckPosAndLoadRead(char ServoID);
char wckActDown(char ServoID);  
char wckPowerDown(void);  
char wckRotation360(char ServoID, char SpeedLevel, char RotationDir);  
void wckSyncPosSend(char LastID, char SpeedLevel, char *TargetArray, char Index);  
char wckBaudrateSet(char ServoID, char NewBaud);  
char wckGainSet(char ServoID, char *NewPgain, char *NewDgain);  
char wckIdSet(char ServoID, char NewId);  
char wckGainRead(char ServoID, char *Pgain, char *Dgain);  
char wckResolSet(char ServoID, char NewResol);  
char wckResolRead(char ServoID);  
char wckOverCTSet(char ServoID, char NewOverCT);  
char wckOverCTRead(char ServoID);  
char wckBoundSet(char ServoID, char *NewLBound, char *NewUBound);  
char wckBoundRead(char ServoID, char *LBound, char *UBound);  

#endif // WCK_H
