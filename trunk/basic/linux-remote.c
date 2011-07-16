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

//#define	DBO(x) {x}
#define	DBO(x) {if (simflg==0) {x}}
extern int simflg;

void initsocket()
{
	// set up I/O
	DBO(printf ("LINUX: init i/o\n");)

	openport();

    //check DCMP version

	wckReadPos(30,0);
	printf ("LINUX: DCMP v=%d.%d\n", response[0], response[1]);

	if (!(response[0]==3 && response[1]>10 ))
	{
		printf ("LINUX: INVALID VERSION\n");

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
		return response[0];
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

