#include <stdio.h>
#include <windows.h>

void basic();

void	initsocket();
int		testsocket(char *echoString);
int		simflg=1;

#define	DBO(x) x


int  PP_mtype=4;

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
	printf ("WIN: Servo Synch Send  %d [%d]\n", LastID, SpeedLevel);
	for (i=Index; i<=LastID; i++)
	{
			char buff[64];
			sprintf(buff, "S:%d:%d$", i, TargetArray[i]);
			testsocket(buff);

			printf ("WIN: Servo [%d] = %d\n", i, TargetArray[i]);
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

int nos;

void PlayPose(int d, int s, int f, unsigned char data[], int n)
{
	int i=0;
	if (n!=0) nos=n; else n=nos;
	printf ("WIN: Playpose  [d=%d , f=%d] :: Data=", d,s);
	for (i=0; i<n; i++)
			printf ("%d,", data[i]);
	printf ("\n");
	Sleep(d);
}

void standup      	(int n)	{printf ("WIN: standup %d\n", n);}
int readservos ()  {nos=20;return nos;}

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

const BYTE basic18[] = { 143, 179, 198, 83, 106, 106, 69, 48, 167, 141, 47, 47, 49, 199, 192, 204, 122, 125};
const BYTE basic16[] = { 125, 179, 199, 88, 108, 126, 72, 49, 163, 141, 51, 47, 49, 199, 205, 205 };

/* sensors */
int z_value=0;
int y_value=0;
int x_value=0;
int gtick=0;
int gVOLTAGE=10000;
int gDistance=35;

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
void lights() {}

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
       printf ("WIN:: Binary mode\n");
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


 