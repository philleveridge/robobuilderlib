//==============================================================================
// edit functions - include file for edit.c
//==============================================================================


typedef struct basic_line {
    int           lineno;
	unsigned int  next;
	unsigned char token;
	unsigned char var;
	int           value;
	char          *text; // rest of line - unproceesed
} line_t;


/*    edit functions   */
extern int     	findln(int lineno);
extern void    	insertln(line_t newline);
extern void    	deleteln(int lineno);
extern void    	clearln();
extern line_t 	readln(char *bp);
extern uint8_t 	nextchar();
extern int     	nxtline;
extern void    	setline(uint8_t);
extern int		firstline();
extern int		getlineno(int p);
extern uint16_t lastline;  // last line added
extern void		readtext(int ln, char *b);
