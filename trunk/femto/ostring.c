#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "win.h"
#endif

#ifdef LINUX
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "linux.h"
#endif

#ifdef AVR
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h> 
#include <avr/interrupt.h>

#include "rprintf.h"
#endif


#include "mem.h"
#include "oobj.h"

extern int dbg;

/**********************************************************/
/*  strings                                               */
/**********************************************************/

char *newstring(char *s)
{
	char *r;
	int n=strlen(s);
	if (dbg) rprintf ("New String [%d - %s]\n",n,s);
	r=(char *)bas_malloc(n*sizeof(char)+1);
	strcpy(r, s);
	return r;
}

char *newstring1(int n)
{
	char *r;
	if (dbg) rprintf ("New String [%d]\n",n);
	r=(char *)bas_malloc(n*sizeof(char)+1);
	return r;
}

void delstring(char *s)
{
	if (s==0) return;
	if (dbg) rprintf ("Del String (%s)\n", s);
	bas_free(s);
}

tOBJ makestring(char *s)
{
	tOBJ r;
	r.type=SYM;
	r.string = newstring(s);
	r.cnt=0;
	r.q=0;
	return r;
}


