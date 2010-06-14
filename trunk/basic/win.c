#include <stdio.h>

/* wck commands */
void wckPosSend()			{}
void wckSetPassive()		{}
void wckGetByte()			{}
void wckFlush()				{}
void wckSendByte()			{}
void wckReInit()			{}
void wckPosRead()			{}
void wckSyncPosSend()		{}

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
void delay_ms(int x)  				{}

void complete_motion()  			{}
void PlaySceneFromBuffer()  		{}
void LoadMotionFromBuffer()  		{}
void GetNextMotionBuffer()  		{}

void SampleMotion(char action)	{printf ("Win:  sample motion %d\n", action);}