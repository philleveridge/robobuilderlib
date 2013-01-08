//==============================================================================
// edit functions - include file for edit.c
//==============================================================================

#ifndef EDIT_H

#define EDIT_H

#ifdef OMNIMA
#define EEPROM_MEM_SZ 	65536 // 64K
#else
#define EEPROM_MEM_SZ 	3072 // 3K
#endif

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
	EXTEND, ON, MATRIX,
	NOTOKENS
	};


enum { 	sVOLT=0, sIR, 
	sKBD, sRND, sSERVO, sTICK, sPORT, sROM, 
	sTYPE, sABS, sIR2ACT, sKIR, sFIND, sCVB2I, 
	sNE, sNS, sMAX, sSUM, sMIN, sNORM, sSQRT, 
	sSIN, sCOS, sIMAX, sHAM, sRANGE, sSIG, sDSIG,
	sSTAND, sZEROS,
	sMIC, sGX, sGY, sGZ, sPSD, 
	sGREY, sTURTLE, sEVENT, sMAP, sSHUF, sSCALE,
#ifdef IMAGE
	sIMR,
	sPLY,
#endif
	NOSPECS
	};



/*    edit functions   */
extern int     	findln(int lineno);
extern void    	insertln(line_t newline);
extern void    	deleteln(int lineno);
extern void    	clearln();
extern line_t 	readln(char *bp);
extern unsigned char 	nextchar();
extern unsigned int nxtline;
extern void    	setline(unsigned int);
extern int		firstline();
extern int		getlineno(int p);
extern unsigned int lastline;  // last line added
extern void		readtext(int ln, unsigned char *b);
extern int		findend();

#endif


