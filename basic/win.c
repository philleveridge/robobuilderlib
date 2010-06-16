#include <stdio.h>
#include <windows.h>

/* wck commands */
void wckPosSend(char ServoID, char Torque, char Position)			{printf ("WIN: Servo Send %d [%d] -> $d\n", ServoID, Torque, Position);}
int  wckPosRead(char ServoID)										{printf ("WIN: Servo Read %d\n", ServoID); return (ServoID<20)?120:-1;}
void wckSetPassive(char ServoID)									{printf ("WIN: Servo Passive %d\n", ServoID); }
void wckSyncPosSend(char LastID, char SpeedLevel, char *TargetArray, char Index)		
{
	int i=0;
	printf ("WIN: Servo Synch Send  %d [%d]\n", LastID, SpeedLevel);
	for (i=Index; i<=LastID; i++)
			printf ("WIN: Servo [%d] = %d\n", i, TargetArray[i]);
}

void wckGetByte()			{}
void wckFlush()				{}
void wckSendByte()			{}
void wckReInit()			{}
void send_bus_str()			{} // support for PIC based Cylon head

int nos;

void PlayPose(int d, int s, unsigned char data[], int n)
{
	int i=0;
	printf ("WIN: Playpose  [%d , %d]\n", d,s);
	for (i=0; i<n; i++)
			printf ("WIN: Data [%d] = %d\n", i, data[i]);
}

void standup      	(int n)	{printf ("WIN: standup %d\n", n);}
void readservos ()  {nos=20;}

/* eeprom */
void			eeprom_read_block (char *b, char *d, int l)		{int i=0; for(i=0; i<l;i++) *b++=*d++;}
unsigned int	eeprom_read_word  (char *p)						{return (*p + (*(p+1)<<8));}
unsigned char	eeprom_read_byte  (char *p) 					{return *p;}
void			eeprom_write_block(char *d, char *b, int l) 	{int i=0; for(i=0; i<l;i++) *b++=*d++;}
void			eeprom_write_word (char *b, unsigned int  w) 	{*b=w%256; *(b+1)=w/256;}
void			eeprom_write_byte (char *b, unsigned char c) 	{*b=c;}

/* sensors */
int z_value=0;
int y_value=0;
int x_value=0;

int irGetByte()			{return -1;}
int adc_volt()			{return 0;}
int adc_psd()			{return 0;}
int tilt_read()			{return 0;}
int adc_mic()			{return 0;}

/* priotf  */
int uartGetByte() 							{return kbhit()?getch():-1; }
void rprintfStrLen(char *p, int s, int l)	{int i; for (i=0; i<l; i++) putchar(*(p+i));}

/* misc */
void delay_ms(int x)  				{Sleep(x);}

void SampleMotion(char action)	{printf ("Win:  sample motion %d\n", action);}