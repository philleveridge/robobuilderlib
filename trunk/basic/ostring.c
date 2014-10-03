#ifdef WIN32

#define _CRT_SECURE_NO_DEPRECATE 
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
	if (dbg) printf ("New String [%d - %s]\n",n,s);
	r=(char *)bas_malloc(n*sizeof(char)+1);
	strcpy(r, s);
	return r;
}

char *newstring1(int n)
{
	char *r;
	if (dbg) printf ("New String [%d]\n",n);
	r=(char *)bas_malloc(n*sizeof(char)+1);
	return r;
}

void delstring(char *s)
{
	if (s==0) return;
	if (dbg) printf ("Del String (%s)\n", s);
	bas_free(s);
}

tOBJ makestring(char *s)
{
	tOBJ r = emptyTPObj(SYM, newstring(s));
	return r;
}

void upperstring(tOBJ r)
{
	if (r.type==SYM)
	{
		char *p=r.string;
		if (p != NULL)
		{
			while (*p !=0) {if (*p>='a' && *p<='z') *p=*p-'a'+'A'; p++;};
		}
	}	
}

tOBJ makenumfstr(char *s)
{
	tOBJ r = emptyObj();
	int n=0;
	float f=0.0, rm=1.0;
	int sgn=1;
	int flt=0;
	if (*s == '-') 
		{sgn=-1; s++;}
	else if (*s == '+') 
		{s++;}

	while (*s != '\0')
	{
		if (*s=='.' && flt==0) 
		{
			s++;
			flt=1;
			f=n;
		}
		if (*s<'0' || *s>'9') 
			break;

		if (flt==0)
		{
			n=n*10+*s-'0';
		}
		else
		{
			rm=rm/10;
			f=f+rm*(*s-'0');
		}
		s++;
	}

	if (flt==1)
	{
		r.floatpoint=f*sgn;
		r.type=FLOAT;
	}
	else
	{
		r.number=n*sgn;
		r.type=INTGR;
	}
	return r;
}

void printstring(FILE *fp, char *cp)
{
	char ch;
	while ( (ch=*cp++) != '\0')
	{
		if ((ch=='\\') && ((*cp)=='n') )
		{ 
			fputc(13,fp); 
			fputc(10,fp); 
			cp++;
			continue;
		}
		fputc(ch,fp);
	}
}

