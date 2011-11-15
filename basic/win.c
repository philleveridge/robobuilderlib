#include <stdio.h>
#include <Windows.h>
#include <direct.h>
#include <conio.h>
#include <math.h>

//#include "win.h"

#define GetCurrentDir _getcwd

extern void basic();

int 	dbg=0;
int		simflg=0;
int		remote=0;

int chargemode()		{return 0;}
int Get_VOLTAGE()		{return 0;}
void send_bus_str()		{} 

#define	DBO(x) {if (simflg==0) {x}}

int nos;

void SendToSoundIC(int n)	{printf ("WIN: Play Sound %d\n", n);}


/* eeprom */
void			eeprom_read_block (BYTE *b, char *d, int l)		{int i=0; for(i=0; i<l;i++) *b++=*d++;}
WORD 	eeprom_read_word  (BYTE *p)
{
	unsigned int r= (BYTE)(*p) ;
	r += (((BYTE)*(p+1))<<8);
	return r;
}
BYTE	eeprom_read_byte  (BYTE *p) 					{return *p;}
void	eeprom_write_block(char *d, BYTE *b, int l) 	{int i=0; for(i=0; i<l;i++) *b++=*d++;}
void	eeprom_write_word (BYTE *b, WORD  w) 	{*b=w%256; *(b+1)=w/256;}
void	eeprom_write_byte (BYTE *b, BYTE c) 			{*b=c;}

/* sensors */
int z_value=0;
int y_value=0;
int x_value=0;
volatile WORD	gtick=0;
int gVOLTAGE=10000;
int gDistance=35;

volatile BYTE	MIC_SAMPLING=1;

BYTE    MIC_LEVEL=0;
WORD    MIC_DLY=0;
BYTE    MIC_STOP=0;
BYTE    MIC_RATE=4;
BYTE    MIC_NOS=32;//SDATASZ;

int sDcnt=0;

unsigned char sData[64];

void sample_sound(int n){printf ("WIN: Sample sound %d\n", n);}
void sound_init()		{printf ("WIN: Sound init\n");}


int adc_mic()			{return 0;}

/* priotf  */
int uartGetByte() 							
{
	return _kbhit()?_getch():-1; 
}


void rprintfStrLen(char *p, int s, int l)	{int i; for (i=0; i<l; i++) putchar(*(p+i));}
void rprintfCRLF()							{printf ("\n");}
void rprintfStr(char *z)					{printf ("%s", z); fflush(stdout);}
void rprintfChar(char c)					{putchar (c); fflush(stdout);}


/* misc */
int delay_ms(int x)  					
{
	Sleep(x);
	return 0;
}

void SampleMotion(char action)			{printf ("Win:  sample motion %d\n", action);}

int   cbyte       (BYTE b) {return b>127?b-256:b;}

int Sqrt(int x)
{
	double f= sqrt((double)x);
	return (int)f;
}

void PSD_off() {}

extern char FIRMWARE[64];  

extern unsigned char BASIC_PROG_SPACE[];

volatile WORD    gMSEC;
volatile BYTE    gSEC;
volatile BYTE    gMIN;
volatile BYTE    gHOUR;

extern WORD psize; // points to next elemt

void binstore()
{
    FILE *fp;
    char *dig="0123456789ABCDEF";
    int i=0;

	if ((fp = fopen("bindata.txt", "w")) == 0)
			return;

	while (i<psize+4)
	{
		BYTE c= BASIC_PROG_SPACE[i++];
		fputc(*(dig+(c/16)),fp);
		fputc(*(dig+(c%16)),fp);
	}


    fclose(fp);
}


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

//4, 8, 16,32,64
void  lights(int n) // power bar meter!
{
	int range[5] = {4,8,16,32,64};
	blights(n, range);
}



void initfirmware() {	
	char  fixb[] = {0, 
		0x31, 0x30, 0x34, 0x31, 0x31, 0x30, 0x30, 0x30, 0x31, 0x30, 0x36, 0x39, 0x30,
		0x00, 0x00, 0x00
		};
	int i;
		
	for (i=0; i<16; i++)
	{
		eeprom_write_byte((BYTE *)(FIRMWARE+i), fixb[i]);
	}

	binmode2();
}


int main(int argc, char *argv[])
{
	initsocket();
	initfirmware();
	basic();
}


 