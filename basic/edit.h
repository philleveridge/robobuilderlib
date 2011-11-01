//==============================================================================
// edit functions - include file for edit.c
//==============================================================================

#define EEPROM_MEM_SZ 	3072 // 3K

typedef struct basic_line {
    int           lineno;
	unsigned int  next;
	unsigned char token;
	unsigned char var;
	int           value;
	char          *text; // rest of line - unproceesed
} line_t;

enum {
	LET=0, FOR, IF, THEN, 
	ELSE, GOTO, PRINT, GET, 
	PUT, END, LIST, XACT, 
	WAIT, NEXT, SERVO, MOVE,
	GOSUB, RETURN, POKE, STAND,
	PLAY, OUT, OFFSET, RUN, I2CO, I2CI,
	STEP, SPEED, MTYPE, LIGHTS,	SORT, FFT,
	SAMPLE, SCALE, DATA,
	SET, INSERT, DELETE, GEN, NETWORK, SELECT,
	NOTOKENS
	};


enum { 	sMIC=0, sGX, sGY, sGZ, sPSD, sVOLT, sIR, 
		sKBD, sRND, sSERVO, sTICK, sPORT, sROM, 
		sTYPE, sABS, sIR2ACT, sKIR, sFIND, sCVB2I, 
		sNE, sNS, sMAX, sSUM, sMIN, sNORM, sSQRT, 
		sSIN, sCOS, sIMAX, sHAM, sRANGE, sSIG, sDSIG,
		sSTAND, sZEROS,
		NOSPECS
	};



/*    edit functions   */
extern int     	findln(int lineno);
extern void    	insertln(line_t newline);
extern void    	deleteln(int lineno);
extern void    	clearln();
extern line_t 	readln(char *bp);
extern uint8_t 	nextchar();
extern uint16_t nxtline;
extern void    	setline(uint16_t);
extern int		firstline();
extern int		getlineno(int p);
extern uint16_t lastline;  // last line added
extern void		readtext(int ln, char *b);
extern int		findend();


