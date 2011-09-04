#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

int fd = -1;

struct termios oldtio,newtio;

#define BAUDRATE B115200
const char *device = "/dev/ttyUSB0";


int response[32];

void openport()
{
	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd == -1) {
	  printf( "failed to open port\n" );
	}

    tcgetattr(fd,&oldtio); /* save current serial port settings */
    bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

    newtio.c_cflag = BAUDRATE | CS8  | CLOCAL ; //| CREAD;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_iflag = IGNPAR;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
}

void writebyte(int b)
{
	char buf[1];
	if(fd == -1) {
	  printf( "open port\n" );
	  return;
	}

	buf[0] = b;
	write(fd, buf, 1);
}

int readbyte()
{
	char buf[1];
	if(fd == -1) {
	  printf( "open port\n" );
	  return -1;
	}

	int cnt=0;

	while (1)
	{
		int b = read(fd, buf, 1);
		if (b>0)
			return buf[0];

		if (b<0)
			return -1;

		if (cnt++>10000) // timeout
			return -1;
	}
}

void closeport()
{
	if(fd == -1) {
	  printf( "open port\n" );
	  return;
	}

    /* restore the old port settings */
    tcsetattr(fd,TCSANOW,&oldtio);
	close(fd);
}

/**************************************************************************************/



/**************************************************************************************/

#define	DBO(x) {if (dbg) {x}}
extern int simflg, dbg;

void initsocket()
{
	// set up I/O
	DBO(printf ("LINUX: init i/o\n");)

	openport();

    //check DCMP version

	wckReadPos(30,0);
	printf ("DCMP v=%d.%d\n", response[0], response[1]);

	if (!(response[0]==3 && response[1]>10 ))
	{
		printf ("Not connected\n");

		closeport();
		simflg=0;
		//exit(1);
	}
}

int testsocket(char *echoString)
{
	/* Dummy - no socket needed */
	return -1;
}

/* wck commands */
void wckPosSend(unsigned char ServoID, char Torque, unsigned char Position)
{
	DBO(printf ("LINUX: Servo Send %d [%d] -> %d\n", ServoID, Torque, Position);)
	wckMovePos(ServoID, Position, Torque);
}

int  wckPosRead(char ServoID)
{
	if (wckReadPos(ServoID, 0)<0)
		return -1;

	DBO(printf ("LINUX: Servo Read %d=%d\n", ServoID, response[1]); )
		return response[1];
}

void wckSetPassive(char ServoID)
{
	DBO(printf ("LINUX: Servo Passive %d\n", ServoID); )
	wckPassive(ServoID);
}

void wckSyncPosSend(char LastID, char SpeedLevel, char *TargetArray, char Index)
{
	int i=0;
	DBO(printf ("LINUX: Servo Synch Send  %d [%d]\n", LastID, SpeedLevel);)

	for (i=Index; i<=LastID; i++)
	{
			wckPosSend(i,SpeedLevel,TargetArray[i]);
	}
}

void wckWriteIO(unsigned char ServoID, unsigned char IO)
{
	DBO(printf ("LINUX: Servo write IO %d=%d\n", ServoID, IO); )
}

/**************************************************************************************

  DCMP commands

 **************************************************************************************/
extern int z_value,y_value,x_value,gDistance;

   int CB2I(int x) {if (x<128) return x; else return x-256;}

   int readXYZ()
   {
	   wckReadPos(30,1);
		y_value = CB2I(response[0]);
		z_value = CB2I(response[1]);
	   wckReadPos(30,2);
		x_value = CB2I(response[0]);
   }

   int readPSD()
   {
	   wckReadPos(30,5);
	   gDistance = CB2I(response[0]);
   }

   int wckReadPos(int id, int d1)
   {
	   response[0]=0;
	   response[1]=0;

	   writebyte(0xFF);
	   id = (5 << 5 | (id % 31));
	   writebyte(id);
	   writebyte(d1);
	   writebyte((id ^ d1) &0x7f);
	   response[0] = readbyte();
	   response[1] = readbyte();
	   return 0;
   }

   int wckPassive(int id)
   {
	   response[0]=0;
	   response[1]=0;

	   writebyte(0xFF);
	   id = (6 << 5 | (id % 31));
	   writebyte(id);
	   writebyte(0x10);
	   writebyte((id ^ 0x10) &0x7f);
	   response[0] = readbyte();
	   response[1] = readbyte();
	   return 0;
   }

   int wckMovePos(int id, int pos, int torq)
   {
	   response[0]=0;
	   response[1]=0;

	   writebyte(0xFF);
	   id  = ((torq % 5) << 5) | (id % 31);
	   pos = pos % 254;
	   writebyte (id);
	   writebyte (pos);
	   writebyte ((id ^ pos) & 0x7f);

	   response[0] = readbyte();
	   response[1] = readbyte();
       return 0;
   }

	void SyncPosSend(int LastID, int SpeedLevel, char *TargetArray, int Index)
	{
		int i = 0;
		int CheckSum = 0;

		writebyte(0xFF);
	    writebyte((SpeedLevel << 5) | 0x1f);
	    writebyte(LastID + 1);

		while (1)
		{
			if (i > LastID) break;
			writebyte(TargetArray[Index * (LastID + 1) + i]);
			CheckSum ^= TargetArray[Index * (LastID + 1) + i];
			i++;
		}
		writebyte(CheckSum & 0x7f);
		return;
	}



	#define BYTE unsigned char

	BYTE cpos[32];
	int offset[32];
	BYTE nos=0;


	int readservos(int n)
	{
		BYTE i;
	        if (n==0) n=31;
		for (i=0; i<n; i++)
		{
			int p = wckPosRead(i);
			if (p<0 || p>255) break;
			cpos[i]=p;
		}
		nos=i;
		return i;
	}

	enum { AccelDecel=0, Accel, Decel, Linear };

	int PP_mtype=Linear;

	double CalculatePos_Accel(int Distance, double FractionOfMove)
	{
		return FractionOfMove * (Distance * FractionOfMove);
	}

	double CalculatePos_Decel(int Distance, double FractionOfMove)
	{
		FractionOfMove = 1 - FractionOfMove;
		return Distance - (FractionOfMove * (Distance * FractionOfMove));
	}

	double CalculatePos_Linear(int Distance, double FractionOfMove)
	{
		return (Distance * FractionOfMove);
	}

	double CalculatePos_AccelDecel(int Distance, double FractionOfMove)
	{
		if ( FractionOfMove < 0.5 )     // Accel:
			return CalculatePos_Accel(Distance /2, FractionOfMove * 2);
		else if (FractionOfMove > 0.5 ) //'Decel:
			return CalculatePos_Decel(Distance/2, (FractionOfMove - 0.5) * 2) + (Distance * 0.5);
		else                            //'= .5! Exact Middle.
			return Distance / 2;
	}

	double GetMoveValue(int mt, int StartPos, int EndPos, double FractionOfMove)
	{
		int Offset,Distance;
		if (StartPos > EndPos)
		{
			Distance = StartPos - EndPos;
			Offset = EndPos;
			switch (mt)
			{
				case Accel:
					return Distance - CalculatePos_Accel(Distance, FractionOfMove) + Offset;
				case AccelDecel:
					return Distance - CalculatePos_AccelDecel(Distance, FractionOfMove) + Offset;
				case Decel:
					return Distance - CalculatePos_Decel(Distance, FractionOfMove) + Offset;
				case Linear:
					return Distance - CalculatePos_Linear(Distance, FractionOfMove) + Offset;
			}
		}
		else
		{
			Distance = EndPos - StartPos;
			Offset = StartPos;
			switch (mt)
			{
				case Accel:
					return CalculatePos_Accel(Distance, FractionOfMove) + Offset;
				case AccelDecel:
					return CalculatePos_AccelDecel(Distance, FractionOfMove) + Offset;
				case Decel:
					return CalculatePos_Decel(Distance, FractionOfMove) + Offset;
				case Linear:
					return CalculatePos_Linear(Distance, FractionOfMove) + Offset;
			}
		}
		return 0.0;
	}

	// Play d ms per step, f frames, from current -> spod
	void PlayPose(int d, int f, int tq, BYTE spod[], int flag)
	{
		int i;

		DBO(printf ("LIN: Playpose  [d=%d , f=%d]\n", d,f);)

		if (flag!=0)
		{
			readservos(0);	// set nos and reads cpos
			nos=flag;
		}

		int dur=d/f;
		if (dur<25) dur=25; //25ms is quickest

		for (i=0; i<f; i++)
		{
			BYTE temp[nos];
			for (int j=0; j<nos; j++)
			{
				//temp[j] = cpos[j] + (float)((i)*intervals[j]+0.5);
				temp[j] = (BYTE)GetMoveValue(PP_mtype, cpos[j], spod[j], (double)i / (double)f);
			}
			wckSyncPosSend(nos-1, tq, temp, 0);
			delay_ms(dur);
		}

		for (i=0; i<nos; i++)
		{
			cpos[i]=spod[i];
		}

		wckSyncPosSend(nos-1, tq, cpos, 0);
		delay_ms(dur);
	}

	const BYTE basic18[] = { 143, 179, 198, 83, 106, 106, 69, 48, 167, 141, 47, 47, 49, 199, 192, 204, 122, 125};
	const BYTE basic16[] = { 125, 179, 199, 88, 108, 126, 72, 49, 163, 141, 51, 47, 49, 199, 205, 205 };
	const BYTE basicdh[] = { 143, 179, 198, 83, 105, 106, 68, 46, 167, 140, 77, 70, 152, 165, 181, 98, 120, 124, 99};

	int dm=0;
	void setdh(int n) {dm=n;}
	void standup (int n)
	{
		if (n<18)
			PlayPose(1000, 10, 4, basic16, 16); //huno basic
		else
		{
			if (dm)
			    PlayPose(1000, 10, 4, basicdh, 18); //huno with hip
			else
			    PlayPose(1000, 10, 4, basic18, 18); //huno with hip
		}
	}




