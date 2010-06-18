#ifdef AVR
#include <avr/io.h>
#include <avr/eeprom.h> 
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "rprintf.h"
#include "math.h"
#else
//windows mods
#include "win.h"
#endif


#include <stdio.h>
#include "edit.h"

extern uint8_t EEMEM BASIC_PROG_SPACE[];  // this is where the tokenised code will be stored
extern int strlen(char *p);

uint16_t psize   =0; // points to next elemt
uint16_t lastline=0; // 
uint16_t pll     =0; // 

//insert newline into BASIC_PROG_SPACE

void insertln(line_t newline)
{
	uint8_t l=0;
	uint8_t nxt=0;
	int srt;

	if (psize==0)
	{
		clearln();
	}

	if ((nxt = findln(newline.lineno)))
	{
		//line already exists
		printf("Edit line\r\n");
	}

	srt=psize;

	eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+psize), newline.lineno);	
	psize+=2;
	eeprom_write_byte(BASIC_PROG_SPACE+psize, newline.token);	
	psize++;		
	eeprom_write_byte(BASIC_PROG_SPACE+psize, newline.var);	
	psize++;	
	eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+psize), newline.value);	
	psize+=2;			

	if (newline.text != 0) l = strlen(newline.text);

	eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+psize), psize+ l + 3);	
	
	psize+=2;

	if (l>0)
	{
		eeprom_write_block(newline.text, BASIC_PROG_SPACE+psize, l);			
		psize+=l;
	}
	eeprom_write_byte(BASIC_PROG_SPACE+psize, 0x0);	// eo string  character
	psize++;

	lastline = psize;
	eeprom_write_byte(BASIC_PROG_SPACE+psize, 0xCC);// terminator character

	if (nxt == 1)
	{
		// pointing to before first line
		// set to this line
		int fl = (int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+1));
		eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+1),  srt);       //top points new first line
		eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+srt+6), fl);	//this now points to old firstline
		//need to terminate list if firstline=lastline
		eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+pll), lastline);	
	}

	if (nxt > 1)
	{
		int n = getlineno(nxt);	

		if (newline.lineno == n)
		{
			int la;
			//delete currentline
			eeprom_write_byte(BASIC_PROG_SPACE+nxt,   0);
			eeprom_write_byte(BASIC_PROG_SPACE+nxt+1, 0);

			// update
			la  = (int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nxt+6));	// start of line after
			// this line to old line next
			eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+nxt+6), srt);

			if (la == srt)
			{
				//this was the last line - so terminator already correct
			}
			else
			{
				eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+srt+6), la);	
				// last lines now to this terminator
				eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+pll), lastline);	
			}
		}
		else
		{
			// insert
			int la = (int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nxt+6));	// start of line after
			//line before [nxt] need to point to [srt] line
			eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+nxt+6), srt);
			//this line to its next line [la]
			eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+srt+6), la);	
			//next line [la] now to point to end 
			eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+la+6), srt + 6 + l + 3);	
		}
	}

	if (nxt==0)
		pll = psize-l-3;
}

int getlineno(int p)
{
	if (p<3) return 0;
	return (int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+p));	
}

void deleteln(int lineno)
{
	int p = findln(lineno);
	if (p>0)
	{
		// unlink item
		eeprom_write_byte(BASIC_PROG_SPACE+p,   0);
		eeprom_write_byte(BASIC_PROG_SPACE+p+1, 0);
	}
}

void clearln()
{
	//write header
	uint8_t data = 0xAA; // start of program byte		
	eeprom_write_byte(BASIC_PROG_SPACE, data);
	//
	// neeed to add top pointer -> 
	eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+1), 3 );	
	psize=3;
	//
	eeprom_write_byte(BASIC_PROG_SPACE+psize, 0xCC);	// terminator character
}

// find lineno
// line exists         :: return ptr to line
// line new at end     :: return ptr to line before
// line new not ar end :: return 0

int findln(int lineno)
{
	int nl  = firstline();
	int prv = 1;
	int lno = 1;

	while (nl<2048)
	{
		lno=eeprom_read_byte(BASIC_PROG_SPACE+nl);					
		if (lno == 0xCC)
		{
			return 0;
		}	
		lno += (eeprom_read_byte(BASIC_PROG_SPACE+nl+1)<<8);	
		
		if (lno == lineno)
		{
			return nl;
		}
				
		if (lno > lineno)
		{
			return prv;
		}

		prv=nl;
		nl=(int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nl+6));	
	}
	return 0;
}

void readtext(int ln, char *b);

int nxtline=0;

line_t readln2(char *bp)
{
	line_t   line;
	line.text = bp;
	lastline=nxtline;

	line.lineno= (int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nxtline));	

	nxtline+=2;
	line.token = eeprom_read_byte(BASIC_PROG_SPACE+nxtline);	
	nxtline++;		
	line.var   = eeprom_read_byte(BASIC_PROG_SPACE+nxtline);	
	nxtline++;	
	line.value = (int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nxtline));	
	nxtline+=2;
	line.next  = (int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nxtline));	
		
	readtext(nxtline+2, line.text);

	nxtline=(int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nxtline));	
	return line;
}

line_t readln(char *bp)
{
	while (1) {
		line_t r = readln2(bp);
		if (r.lineno!=0 || nextchar()==0xcc)
			return r;
	}
}

uint8_t nextchar()
{
	return eeprom_read_byte(BASIC_PROG_SPACE+nxtline);	// terminator character ?
}

int firstline()
{
	nxtline = (int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+1));	 //top
	return nxtline;
}

void setline(uint8_t p)
{
	nxtline =p;
}

void readtext(int ln, char *b)
{
	int i = 0;
	//eeprom_read_block(bp, BASIC_PROG_SPACE+nxtline+2, l);	
	for (i=0; i<100;i++) // MAX_LINE
	{
		b[i] =eeprom_read_byte((uint8_t *)(BASIC_PROG_SPACE+ln+i));	
	    if (b[i]==0) break;
	}
}