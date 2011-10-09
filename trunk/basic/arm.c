#include "cor_init.h"
#include "cor_hwlib.h"
#include "cor_wrflash.h"

#include "mem.h"		// not used here, included only for reference
#include "string.h"
#include "printf.h"
#include "constants.h"
#include "coridium.h"

#include "arm.h"

extern void basic();
extern void basic_zero();

int 	dbg=0;
int	simflg=0;
int	remote=1;
int  	irf=0;

#define	DBO(x) {if (dbg) {x}}


int uartGetByte() {
	return getc();
}

void  lights(int n) {
	DBO(printf ("ARM: Lights %d\n",n);)
}

void  blights(int n, int *a) {
	DBO(printf ("ARM: Lights %d [%d,%d,%d,%d,%d]\n",n,a[0], a[1], a[2], a[3], a[4]);)
}


void wckGetByte()			{}
void wckFlush()				{}
void wckSendByte()			{}
void wckReInit()			{}
void send_bus_str()			{} // support for PIC based Cylon head

BYTE cpos[32];
unsigned char nos;



void SendToSoundIC(int n)	{DBO(printf ("ARM: Play Sound %d\n", n);)}

/* eeprom */
void	eeprom_read_block (BYTE *b, char *d, int l)	{int i=0; for(i=0; i<l;i++) *b++=*d++;}
WORD	eeprom_read_word  (BYTE *p)
{
	WORD r= (BYTE)(*p) ;
	r += (((BYTE)*(p+1))<<8);
	return r;
}
BYTE	eeprom_read_byte  (BYTE *p) 			{return *p;}
void	eeprom_write_block(char *d, BYTE *b, int l) 	{int i=0; for(i=0; i<l;i++) *b++=*d++;}
void	eeprom_write_word (BYTE *b, WORD w) 		{*b=w%256; *(b+1)=w/256;}
void	eeprom_write_byte (BYTE *b, BYTE c) 		{*b=c;}

/* sensors */
int z_value=0;
int y_value=0;
int x_value=0;
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

void PSD_off() {}

void sample_sound(int n){DBO(printf ("ARM: Sample sound %d\n", n);)}
void sound_init()		{DBO(printf ("ARM: Sound init\n");)}
int Get_VOLTAGE()		{return 0;}


extern int readXYZ();

void  Acc_init    (void) {}

int adc_mic()			{return 0;}

int Sqrt(int x) {return 0;}



/* printf  */

void rprintfStrLen(char *p, int s, int l)	{int i; for (i=0; i<l; i++) putchar(*(p+i));}
void rprintfCRLF()				{printf ("\n");}
void rprintfStr(char *z)			{printf ("%s", z); } //fflush(stdout);}
void rprintfChar(char c)			{putchar (c); } //fflush(stdout);}

/* misc */

void delay_ms(int ms)  					
{
    	int save_time;  // must be signed to handle roll-over  
	ms=ms*1000;
    	save_time = TIMER;
    	while ((TIMER - save_time) < ms) ; 
}

void chargemode() {DBO(printf ("ARM: chargemode\n"); )}


void  I2C_read    (int addr, int ocnt, BYTE * outbuff, int icnt, BYTE * inbuff) 
{
	printf ("ARM: I2C read %d\n", addr);
}
int   I2C_write   (int addr, int ocnt, BYTE * outbuff)  
{
	printf ("ARM: I2C write %d\n", addr);
	return 0;
}
int   cbyte       (BYTE b) {return b>127?b-256:b;}


extern char FIRMWARE[64];  

extern unsigned char BASIC_PROG_SPACE[];

volatile WORD    gMSEC;
volatile BYTE    gSEC;
volatile BYTE    gMIN;
volatile BYTE    gHOUR;
volatile WORD    gSEC_DCOUNT;
volatile WORD    gMIN_DCOUNT;


extern uint16_t psize; // points to next elemt

void binstore()
{
}

void binmode()
{
}

void PlayMotion(int Action) {}


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
}

/***********************
 *
 *
 * *********************/

int main(void)
{
	int lf=0;
	int i=0;

	init_coridium();		// REQUIRED OPERATION

	PORTA=3;

	printf("Running on ARM M3-Cortex ...\n");

	initfirmware();
	basic_zero();
	if (lf)
		binmode();

	basic();

}

 
