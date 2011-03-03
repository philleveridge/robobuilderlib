#include <stdio.h>
#include <windows.h>
#include <direct.h>

#include "Main.h"
#include "win.h"

#define GetCurrentDir _getcwd

void basic();

void	initsocket();
int		testsocket(char *echoString);
int		simflg=1;

#define	DBO(x) {if (simflg==0) {x}}

int  PP_mtype=4;

void  lights(int n) {
	DBO(printf ("WIN: Lights %d\n",n);)
}

void  blights(int n, int *a) {
	DBO(printf ("WIN: Lights %d [%d,%d,%d,%d,%d]\n",n,a[0], a[1], a[2], a[3], a[4]);)
}

/* wck commands */
void wckPosSend(unsigned char ServoID, char Torque, unsigned char Position)			
{
	char buff[64];
	sprintf(buff, "S:%d:%d$", ServoID, Position);
	testsocket(buff);

	DBO(printf ("WIN: Servo Send %d [%d] -> %d\n", ServoID, Torque, Position);)
}
int  wckPosRead(char ServoID)										
{
	int r;
	char buff[64];
	sprintf(buff, "R:%d$", ServoID);
	r = testsocket(buff);

	DBO(printf ("WIN: Servo Read %d\n", ServoID); )
	return r;
}
void wckSetPassive(char ServoID)									
{
	char buff[64];
	sprintf(buff, "P:%d$", ServoID);
	testsocket(buff);

	DBO(printf ("WIN: Servo Passive %d\n", ServoID); )
}

void wckSyncPosSend(char LastID, char SpeedLevel, char *TargetArray, char Index)		
{
	int i=0;
	DBO(printf ("WIN: Servo Synch Send  %d [%d]\n", LastID, SpeedLevel);)

	// convert this to "Y:%d...:%d"
	//tbd
	for (i=Index; i<=LastID; i++)
	{
			wckPosSend(i,SpeedLevel,TargetArray[i]);
	}
}

void wckGetByte()			{}
void wckFlush()				{}
void wckSendByte()			{}
void wckReInit()			{}
void send_bus_str()			{} // support for PIC based Cylon head

void wckWriteIO(unsigned char ServoID, unsigned char IO)    
{
	char buff[64];
	sprintf(buff, "O:%d:%d$", ServoID, IO);
	testsocket(buff);
	DBO(printf ("WIN: Servo write IO %d=%d\n", ServoID, IO); )
}

BYTE cpos[32];
int nos;

const BYTE basic18[] = { 143, 179, 198, 83, 106, 106, 69, 48, 167, 141, 47, 47, 49, 199, 192, 204, 122, 125};
const BYTE basic16[] = { 125, 179, 199, 88, 108, 126, 72, 49, 163, 141, 51, 47, 49, 199, 205, 205 };

void PlayPose(int d, int s, int f, unsigned char data[], int n)
{
	int i=0;
	if (n!=0) nos=n; else n=nos;
	DBO(printf ("WIN: Playpose  [d=%d , f=%d] :: Data=", d,s);)
	for (i=0; i<n; i++)
	{
			DBO(printf ("%d,", data[i]);)

	}
	DBO(printf ("\n");)
	if (simflg!=0)
	{
		wckSyncPosSend(n-1,0,data, 0);
	}
	Sleep(d);
}

void standup      	(int n)	
{
	printf ("WIN: standup %d\n", n);
	if (n==16)
	{
		PlayPose(1000,10,1,basic16,16);
	}
	else
	{
		PlayPose(1000,10,1,basic18,18);
	}
}
int readservos ()  {
	int i=0;
	nos=20;
	for (i=0; i<nos; i++)
		cpos[i] = wckPosRead(i);	
	return nos;
}

void SendToSoundIC(int n)	{printf ("WIN: Play Sound %d\n", n);}

/* eeprom */
void			eeprom_read_block (char *b, char *d, int l)		{int i=0; for(i=0; i<l;i++) *b++=*d++;}
unsigned int	eeprom_read_word  (char *p)						
{
	unsigned int r= (unsigned char)(*p) ;
	r += (((unsigned char)*(p+1))<<8);
	return r;
}
unsigned char	eeprom_read_byte  (char *p) 					{return *p;}
void			eeprom_write_block(char *d, char *b, int l) 	{int i=0; for(i=0; i<l;i++) *b++=*d++;}
void			eeprom_write_word (char *b, unsigned int  w) 	{*b=w%256; *(b+1)=w/256;}
void			eeprom_write_byte (char *b, unsigned char c) 	{*b=c;}

/* sensors */
int z_value=0;
int y_value=0;
int x_value=0;
int gtick=0;
int gVOLTAGE=10000;
int gDistance=35;

BYTE	MIC_SAMPLING=1;

BYTE    MIC_LEVEL=0;
WORD    MIC_DLY=0;
BYTE    MIC_STOP=0;
BYTE    MIC_RATE=4;
BYTE    MIC_NOS=SDATASZ;

int sDcnt=0;

unsigned char sData[64];
int offset[32];

void sample_sound(int n){printf ("WIN: Sample sound %d\n", n);}
void sound_init()		{printf ("WIN: Sound init\n");}
int Get_VOLTAGE()		{return 0;}
int irGetByte()			
{
	char buff[64];
	if (simflg==0) return 0;
	sprintf(buff, "IR$");
	return testsocket(buff);
}

int Get_AD_PSD()		
{
	char buff[64];
	if (simflg==0) return 0;
	sprintf(buff, "PSD$");
	gDistance = testsocket(buff);
	return 0;
}

void  Acc_init    (void) {}
void  Acc_GetData ()			
{
	char buff[64];
	if (simflg==0) return;

	sprintf(buff, "X$");
	x_value = testsocket(buff);
	sprintf(buff, "Y$");
	y_value = testsocket(buff);
	sprintf(buff, "Z$");
	z_value = testsocket(buff);
	return;
}
int adc_mic()			{return 0;}

void leds_buttons()
{
	char buff[64];
	int  v;

	if (simflg==0) return;

	v=  ((PORTA&(1<<6) != 0)?  1:0) + //  PA6 RUN G
		((PORTA&(1<<5) != 0)?  2:0) + //  PA5 RUN b
		((PORTC&(1<<7) != 0)?  4:0) + //  PC7 PWR R
		((PORTG&(1<<2) != 0)?  8:0) + //  PG2 PWR G 
		((PORTA&(1<<7) != 0)? 16:0) + //  PA7 ERROR R
		((PORTA&(1<<3) != 0)? 32:0) + //  PA3 PF1 R			
		((PORTA&(1<<2) != 0)? 64:0) + //  PA2 PF1 B
		((PORTA&(1<<4) != 0)?128:0);  //  PA4 Pf2 O


	sprintf(buff, "H:%d",v);
	v = testsocket(buff);

	//	input; PA0=PF1, PA1=PF2
	if (v&1==1) PORTA |= 1;
	if (v&2==2) PORTA |= 2;
}

/* priotf  */
int uartGetByte() 							{return kbhit()?getch():-1; }
void rprintfStrLen(char *p, int s, int l)	{int i; for (i=0; i<l; i++) putchar(*(p+i));}
void rprintfCRLF()							{printf ("\n");}

/* misc */
void delay_ms(int x)  					
{//printf ("Win:  wait %d\n",x); 
Sleep(x);
}

void SampleMotion(char action)			{printf ("Win:  sample motion %d\n", action);}
void PlayMotion (char action, int f)	{printf ("Win:  Play %d\n", action);}

void  I2C_read    (int addr, int ocnt, BYTE * outbuff, int icnt, BYTE * inbuff) 
{
	printf ("WIN: I2C read %d\n", addr);
}
int   I2C_write   (int addr, int ocnt, BYTE * outbuff)  
{
	printf ("WIN: I2C write %d\n", addr);
	return 0;
}
int   cbyte       (BYTE b) {return b>127?b-256:b;}


extern char FIRMWARE[64];  

extern unsigned char BASIC_PROG_SPACE[];

int binmode2()
{
       FILE *fp;
       char ch;
       int i=0,n=0;

	   if ((fp = fopen("bindata.txt", "r")) == 0)
			return -1;

       while ((ch=fgetc(fp)) != EOF)
       {

              if (ch >= '0' && ch <= '9') n=ch-'0';
              if (ch >= 'A' && ch <= 'F') n=ch-'A'+10;
              ch=fgetc(fp);
              n = n<<4;
              if (ch >= '0' && ch <= '9') n += (ch-'0');
              if (ch >= 'A' && ch <= 'F') n += (ch-'A'+10);
              BASIC_PROG_SPACE[i++] = (unsigned char)n;

       }
       fclose(fp);
	   return 0;
}

void binmode()
{
	char cCurrentPath[FILENAME_MAX];

	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
    {
     return;
    }

    printf ("WIN:: Binary mode (%s)\n", cCurrentPath);
	if (binmode2()<0)
		printf ("? can't find file - bindata.txt\n");
    printf ("Loaded - bindata.txt\n");
}


void initfirmware() {	
	char  fixb[] = {0, 
		0x31, 0x30, 0x34, 0x31, 0x31, 0x30, 0x30, 0x30, 0x31, 0x30, 0x36, 0x39, 0x30,
		0x00, 0x00, 0x00
		};
	int i;
		
	for (i=0; i<16; i++)
	{
		eeprom_write_byte(FIRMWARE+i, fixb[i]);
	}

	binmode2();
}


int main(int argc, char *argv[])
{
	initsocket();
	initfirmware();
	basic();
}


 