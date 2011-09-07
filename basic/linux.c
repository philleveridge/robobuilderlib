#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <termio.h>
#include <fcntl.h>

#include "main.h"
#include "linux.h"

#define GetCurrentDir _getcwd

void basic();

int 	dbg=0;
int		simflg=0;
int		remote=0;

#define	DBO(x) {if (dbg) {x}}

struct termio savetty;
struct termio newtty;  /* terminal with changed mode */

int uartGetByte() {
	char buf[1];
    if (read(0, buf, 1)>0)
    {/* read one char at a time */
		if (buf[0]==10) buf[0]=13;
		return buf[0];
    }
    return -1;
}

int initIO() {

  void sigcatch();       /* what to do if a signal is caught */
  int nrd;               /* number of bytes read */
  int i;

  signal(SIGINT,sigcatch); /* if a ctrl-C is received, do this */

  if (ioctl(0,TCGETA,&savetty) == -1)    { /* save the original terminal */
    printf("ioctl failed: not a tty\n");
    exit(1);
  }

  /* make a copy of the original terminal */
  newtty = savetty;
  newtty.c_lflag &= ~ICANON;   /* turn off canonical mode */
  newtty.c_lflag &= ~ECHO;     /* turn off echoing */
  newtty.c_cc[VMIN] = 1;       /* minimum 1 chars */
  newtty.c_cc[VTIME] = 0;    /* 10 sec interval */

  if (ioctl(0,TCSETAF,&newtty) == -1)    { /* setting the new terminal mode */
    printf("cannot put tty into raw mode \n");
    exit(1);
  }

  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);       // make the reads non-blocking

}

/* interrupt handler on ctrl-C */
void sigcatch() {
  ioctl(0,TCSETAF,&savetty); /* restore the original terminal */
  printf("Exit - you typed control - C\n");
  exit(1);
}


//int  PP_mtype=4;

void  lights(int n) {
	DBO(printf ("LIN: Lights %d\n",n);)
}

void  blights(int n, int *a) {
	DBO(printf ("LIN: Lights %d [%d,%d,%d,%d,%d]\n",n,a[0], a[1], a[2], a[3], a[4]);)
}


void wckGetByte()			{}
void wckFlush()				{}
void wckSendByte()			{}
void wckReInit()			{}
void send_bus_str()			{} // support for PIC based Cylon head

BYTE cpos[32];
int nos;


void Sleep(long ms)
{
	usleep(ms*1000);
}

void SendToSoundIC(int n)	{DBO(printf ("LIN: Play Sound %d\n", n);)}

/* eeprom */
void			eeprom_read_block (unsigned char *b, char *d, int l)		{int i=0; for(i=0; i<l;i++) *b++=*d++;}
unsigned int	eeprom_read_word  (unsigned char *p)
{
	unsigned int r= (unsigned char)(*p) ;
	r += (((unsigned char)*(p+1))<<8);
	return r;
}
unsigned char	eeprom_read_byte  (unsigned char *p) 					{return *p;}
void			eeprom_write_block(char *d, unsigned char *b, int l) 	{int i=0; for(i=0; i<l;i++) *b++=*d++;}
void			eeprom_write_word (unsigned char *b, unsigned int  w) 	{*b=w%256; *(b+1)=w/256;}
void			eeprom_write_byte (unsigned char *b, unsigned char c) 	{*b=c;}

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

void sample_sound(int n){DBO(printf ("LIN: Sample sound %d\n", n);)}
void sound_init()		{DBO(printf ("LIN: Sound init\n");)}
int Get_VOLTAGE()		{return 0;}
int irGetByte()			
{
	char buff[64];
	if (simflg==0)
	{
		return -1;
	}
	sprintf(buff, "IR$");
	return testsocket(buff);
}

int Get_AD_PSD()		
{
	char buff[64];
	if (simflg==0)
	{
		readPSD();
		return;
	}
	sprintf(buff, "PSD$");
	gDistance = testsocket(buff);
	return 0;
}

extern int readXYZ();

void  Acc_init    (void) {}
void  Acc_GetData ()			
{
	char buff[64];
	if (simflg==0)
	{
		readXYZ();
		return;
	}

	sprintf(buff, "X$");
	x_value = testsocket(buff);
	sprintf(buff, "Y$");
	y_value = testsocket(buff);
	sprintf(buff, "Z$");
	z_value = testsocket(buff);
	return;
}
int adc_mic()			{return 0;}

int sqrt(int x) {return 0;}

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
	//v = testsocket(buff);

	//	input; PA0=PF1, PA1=PF2
	if (v&1==1) PORTA |= 1;
	if (v&2==2) PORTA |= 2;
}

/* priotf  */

void rprintfStrLen(char *p, int s, int l)	{int i; for (i=0; i<l; i++) putchar(*(p+i));}
void rprintfCRLF()							{printf ("\n");}
void rprintfStr(char *z)					{printf ("%s", z); fflush(stdout);}
void rprintfChar(char c)					{putchar (c); fflush(stdout);}

/* misc */
void delay_ms(int x)  					
{
	Sleep(x);
}

void chargemode() {DBO(printf ("LIN: chargemode\n"); )}

void SampleMotion(char action)			{DBO(printf ("LIN:  sample motion %d\n", action);)}


void  I2C_read    (int addr, int ocnt, BYTE * outbuff, int icnt, BYTE * inbuff) 
{
	printf ("LIN: I2C read %d\n", addr);
}
int   I2C_write   (int addr, int ocnt, BYTE * outbuff)  
{
	printf ("LIN: I2C write %d\n", addr);
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

int binmode2()
{
       FILE *fp;
       int ch;
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

extern uint16_t psize; // points to next elemt

void binstore()
{
    FILE *fp;
    int ch;
    int i=0,n=0;
    char *dig="0123456789ABCDEF";

	if ((fp = fopen("bindata.txt", "w")) == 0)
			return;

	while (i<psize)
	{
		unsigned char c= BASIC_PROG_SPACE[i++];
		fputc(*(dig+(c/16)),fp);
		fputc(*(dig+(c%16)),fp);
	}


    fclose(fp);
}

void binmode()
{
	char cCurrentPath[FILENAME_MAX];

	//if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
    //{
    // return;
   // }

    DBO(printf ("LIN:: Binary mode (%s)\n", cCurrentPath);)
	if (binmode2()<0)
	{
		printf ("? can't find file - bindata.txt\n");
		basic_clear();
	}
	else
	{
		printf ("Loaded - bindata.txt\n");
	}
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

/***********************
 *
 *
 * *********************/


#include <pthread.h>
#include <stdio.h>

volatile WORD	gtick;
volatile WORD	mstimer;

/* Emulates the timer interupt */
void *monitor_proc(void *arg)
{
	char *str;

	str=(char*)arg;

	while(1) // forever
	{
		usleep(940); //every millisec

		// 1ms
		gtick++;
		if (mstimer>0) mstimer--;

	    if(++gMSEC>999)
	    {
			// 1s
	        gMSEC=0;

	        if(gSEC_DCOUNT > 0)	gSEC_DCOUNT--;

	        if(++gSEC>59)
	        {
				// 1m
	            gSEC=0;
				if(gMIN_DCOUNT > 0)	gMIN_DCOUNT--;

	           if(++gMIN>59)
	           {
					// 1h
	                gMIN=0;
	                if(++gHOUR>23)
	                    gHOUR=0;
	            }
	        }
	    }
	}

}


int main(int argc, char *argv[])
{
	int lf=0;
	PORTA=3;

	for (int i=1; i<argc; i++)
	{
		if (!strcmp(argv[i],"DEBUG"))
			dbg=1;

		if (!strcmp(argv[i],"SIM"))
			simflg=1;

		if (!strcmp(argv[i],"REMOTE"))
			remote=1;

		if (!strcmp(argv[i],"LOAD"))
			lf=1;
	}

	printf("Running Unix emulator ...\n");
	initsocket();
	initIO();
	initfirmware();
	if (lf)
		binmode();
	else
		basic_clear();

	pthread_t pth;	// this is our thread identifier

	pthread_create(&pth,NULL,monitor_proc,"TIMER0");

	basic();

	pthread_join(pth,NULL);
}


 
