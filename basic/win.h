//windows mods

#define uint16_t unsigned int
#define uint8_t  unsigned char
#define EEMEM
#define prog_char char
#define PINA 1
#define PINB 2
#define PINC 3
#define PIND 4
#define PINE 5
#define PINF 6
#define PING 7

char _port[8];

#define PORTA _port[1]
#define PORTB _port[2]
#define PORTC _port[3]
#define PORTD _port[4]
#define PORTE _port[5]
#define PORTF _port[6]
#define PORTG _port[7]

#define rprintf			printf 
#define rprintfChar		putchar 
#define rprintfStr(z)	printf("%s", z) 

//from win.c
extern void			eeprom_read_block (unsigned char *b, char *d, int l);
extern uint16_t 	eeprom_read_word  (unsigned char *p);
extern uint8_t		eeprom_read_byte  (unsigned char *p);
extern void			eeprom_write_block(char *d, unsigned char *b, int l);
extern void			eeprom_write_word (unsigned char *b, unsigned int  w);
extern void			eeprom_write_byte (unsigned char *b, unsigned char c);
extern int			uartGetByte();
extern void			rprintfStrLen(char *p, int s, int l);

extern void			initfirmware();
extern void			binmode();

const unsigned char  basic18[];
const unsigned char  basic16[];
int  offset[];
void PlayPose(int d, int s, int f, unsigned char data[], int n);