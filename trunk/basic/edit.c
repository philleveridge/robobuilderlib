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

uint16_t psize=0; // pints to next elemt

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

	eeprom_write_byte(BASIC_PROG_SPACE+psize, 0xCC);// terminator character

	if (nxt == 1)
	{
		// top - updaing first line
		int n = (int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nxt));	
		if (n == newline.lineno)
		{
			//updaing first line
		}
		else if (n == newline.lineno)
		{
			//inserting before first line
		}
		else
		{
			//inserting after first line
		}
	}

	if (nxt > 1)
	{
		int n = (int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nxt));	

		if (newline.lineno == n)
		{
			// update
			int la  = (int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nxt+6));	// start of line after
			int la2 = (int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+la +6));	// start of line after
			// line before to point to this line
			eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+nxt+6), srt);	//ok
			// this line to old line next
			eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+srt+6), la2);	
			// last lines now to this terminator
		}
		else
		{
			// insert
			int la = (int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nxt+6));	// start of line after
			//line before [nxt] need to point to [srt] line
			eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+nxt+6), srt);	//ok
			//this line to its next line [la]
			eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+srt+6), la);	
			//next line [la] now to point to end 
			eeprom_write_word((uint16_t *)(BASIC_PROG_SPACE+la+6), srt + 6 + l + 3);	
		}
	}		
}

void deleteln(int lineno)
{
	int p = findln(lineno);
	if (p>0)
	{
		// unlink item
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
	int nl=firstline();
	int lno=1;
	int prv=1;

	while (lno != 0)
	{
		uint8_t b;
		lno=eeprom_read_byte(BASIC_PROG_SPACE+nl);					
		if (lno == 0xCC)
		{
			return 0;
		}	
		lno += (eeprom_read_byte(BASIC_PROG_SPACE+nl+1)<<8);	
		
		if (lno >= lineno)
		{
			return prv;
		}
		prv=nl;
		nl=(int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nl+6));	
	}
	return 0;
}


int nxtline=0;

line_t readln(char *bp)
{
	int i;
	line_t   line;
	uint8_t l, tmp=0;
	line.text = bp;


	line.lineno=(int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nxtline));	
	nxtline+=2;
	line.token=eeprom_read_byte(BASIC_PROG_SPACE+nxtline);	
	nxtline++;		
	line.var=eeprom_read_byte(BASIC_PROG_SPACE+nxtline);	
	nxtline++;	
	line.value=(int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nxtline));	
	nxtline+=2;
		
	//eeprom_read_block(bp, BASIC_PROG_SPACE+nxtline+2, l);	
	for (i=0; i<64;i++)
	{
		line.text[i] =eeprom_read_byte((uint8_t *)(BASIC_PROG_SPACE+nxtline+2+i));	
	    if (line.text[i]==0) break;
	}

	nxtline=(int)eeprom_read_word((uint16_t *)(BASIC_PROG_SPACE+nxtline));	

	return line;
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