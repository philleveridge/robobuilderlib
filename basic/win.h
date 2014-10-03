//windows mods
#ifndef WIN_H
#define WIN_H

#include "macro.h"

#define PARSE


#define uint16_t unsigned int
#define uint8_t  unsigned char

//#define WORD  unsigned int
//#define BYTE  unsigned char

#define EEMEM
#define prog_char char
#define PINA _port[0]
#define PINB _port[1]
#define PINC _port[2]
#define PIND _port[3]
#define PINE _port[4]
#define PINF _port[5]
#define PING _port[6]

char _port[8];

#define PORTA _port[0]
#define PORTB _port[1]
#define PORTC _port[2]
#define PORTD _port[3]
#define PORTE _port[4]
#define PORTF _port[5]
#define PORTG _port[6]

//#define SDATASZ 128
#define RUN_LED1_ON
#define RUN_LED1_OFF

#define rprintf			printf 
//#define rprintfChar		putchar 
//#define rprintfStr(z)	printf("%s", z) 

//from win.c
extern WORD			eeprom_read_word  (BYTE *p);
extern BYTE			eeprom_read_byte  (BYTE *p);
extern void			eeprom_write_block(char *d, BYTE *b, int l);
extern void			eeprom_write_word (BYTE *b, WORD  w);
extern void			eeprom_write_byte (BYTE *b, BYTE c);
extern int			uartGetByte();
extern void			rprintfStrLen(char *p, int s, int l);

extern void			initfirmware();
extern void			binmode();

extern const unsigned char  basic18[];
extern const unsigned char  basic16[];

int  offset[];
extern int PP_mtype;
void PlayPose(int d, int s, int f, unsigned char data[], int n);

extern int z_value;
extern int y_value;
extern int x_value;
extern int gVOLTAGE;
extern int gDistance;

extern volatile WORD    gMSEC;
extern volatile BYTE    gSEC;
extern volatile BYTE    gMIN;
extern volatile BYTE    gHOUR;
//extern volatile WORD    gSEC_DCOUNT;
//extern volatile WORD    gMIN_DCOUNT;

extern volatile BYTE	MIC_SAMPLING;


extern int 	dbg;
extern int	simflg;
extern int	remote;

extern volatile WORD	gtick;
extern volatile WORD	mstimer;

#define rprintfProgStr	printf
#define PSTR(a)			a
#define strcmp_P		strcmp
#define strncmp_P		strncmp
#define strlen_P		strlen

extern void rprintfCRLF();
extern void rprintfStr(char *z);
extern void rprintfChar(char c);
//extern int  uartGetByte() ;

#endif