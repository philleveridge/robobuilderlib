#ifdef AVR
#include <avr/eeprom.h> 
#include <avr/pgmspace.h>

#include "global.h"
#include "main.h"
#include "macro.h"

#include "adc.h"
#include "ir.h"
#include "accelerometer.h"
#include "wckmotion.h"
#include "rprintf.h"
#endif

#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include "win.h"
#endif

#ifdef LINUX
#include "linux.h"
#endif

#include "edit.h"
#include "express.h"
#include "functions.h"
#include "lists.h"


extern unsigned char	cpos[];

extern int		BasicErr;
extern BYTE		sData[];
extern int 		sDcnt;
extern BYTE		nos;
extern volatile BYTE	MIC_SAMPLING;
extern volatile WORD	gtick;

extern int dbg;

int scene[SCENESZ];	  	// generic/current (!) array
int nis=0;        		// number of item isn array

static 		int  listmem	[LISTMEM];
static 		char names	[MAXLIST];
static unsigned int  len	[MAXLIST];
static unsigned int  lists	[MAXLIST];

static int  nol=0;
static int  eol=0;

int list_eval(char, char *p, int);

void listinit()
{
	nol=0;
	eol=0;
}

void listdump()
{
	int i,j;
	for (i=0;i<nol;i++)
	{
		rprintf ("%c [%3d]", names[i], len[i]);
		for (j=0;j<len[i]; j++)
			rprintf (" %d", listmem[lists[i]+j]);
		rprintf ("\n");
	}

	rprintf ("! [%3d]",nis);
	for (j=0;j<nis; j++)
		rprintf (" %d", scene[j]);
	rprintf ("\n");
}

int listexist(char ln)
{
	int i;

	if (ln<'A' || ln>'Z')
		return -1;

	for (i=0; i<nol; i++)
	{
		if (ln==names[i])
			break;
	}
	if (i==nol)
		return -1;
	else
		return i;
}

int listsize(char ln)
{
	int n;
	if (ln=='!')
		return nis;

	n=listexist(ln);
	if (n<0) 
		return 0;
	else
		return len[n];
}

int *listarray(char ln)
{
	int n;
	if (ln=='!')
		return &scene[0];

	n=listexist(ln);
	if (n<0) 
		return (int *)0;
	else
		return &listmem[lists[n]];
}


void listdestroy(char ln)
{
	int t,i,li,sz;
	if (dbg) {rprintf("Destroy %c\n", ln); }

	if (ln=='!')
		return;

	li=listexist(ln);
	if (li<0) return;
	if (li==nol-1)
		sz=eol-lists[li];
	else
		sz=lists[li+1]-lists[li];

	t=lists[li];

	for (i=li; i<nol-1; i++)
	{
		names[i] = names[i+1];
		len[i]   = len[i+1];
		lists[i] = lists[i+1]-sz; 
	}

	names[i] = '\0';
	len[i]   = 0;
	lists[i] = 0;

	for (i=t; i<eol; i++)
	{
		listmem[i] = listmem[i+sz];
	}
	nol--;
	eol=eol-sz;
}

int listcreate(char ln, int size, int type)
{
	int t=listexist(ln);

	if (dbg) {rprintf("Create %c %d\n", ln, size); }


	if (t>=0 && len[t]==size)
		return t;

	listdestroy(ln); // remove if already exists

	if (nol==(MAXLIST-1))
		return -1;

	if (dbg) {rprintf("Create -- %d %d\n", eol+size, LISTMEM); }

	if (eol + size < LISTMEM)
	{
		names[nol] = ln;
		len[nol]   = size;
		lists[nol] = eol; // this need setting to next free location
		eol += size;

//printf("DBG:: list create %d %d %d\n", nol,eol,size);
	}
	nol++;
	return nol-1;
}

int listreadi(int l, int n)
{
	if (l== -1 && n>=0 && n<nis)
		return scene[n];

	if (l<0 || n<0 || n>=len[l] )
		return 0;
	else
		return listmem[lists[l]+n];
}

int listread(char ln, int n)
{
	if (ln=='!')
		return scene[n];
	return listreadi(listexist(ln),n);
}

int listreadc(char ln)
{
	int l,i;
	if (ln=='!')
		return nis;
	l=listexist(ln);
	if (l<0)
	{
		nis=0;
		return 0;
	}
	for (i=0; i<len[l]; i++)
		scene[i]=listreadi(l,i);
	nis=len[l];
	return nis;
}

void listwritei(int ind, int n, int v)
{
//if (dbg) {rprintf("List write %d %d %d\n", ind, n, v); }

	if (ind == -1 && n>=0 && n<=SCENESZ)
		scene[n]=v;
	if (ind >=0 && n>=0 && n<len[ind] )
		listmem[lists[ind]+n]=v;
}

void listwrite(char ln, int n, int v)
{
	if (ln=='!')
	{
		scene[n]=v;
	}
	else
	{
		listwritei(listexist(ln), n, v);
	}
}

/*
List Program
10 LIST A = 5,1,2,3,4,5
20 PRINT @A
30 DELETE @A,2,3
40 PRINT @A
50 INSERT @A,2,7
60 PRINT @A
70 SET @A,1,8
80 PRINT @A
: r
Run Program
1,2,3,4,5
1,2,5
1,2,7,5
1,8,7,5
*/

void listset(char ln, int ind, long v, int insrtflg)
// i.e. SET    I,V
//      INSERT I,V
// current array ![I]=V
{
	int i, *array, sz, msz, l=0;

	if (ln=='!')
	{
		sz=nis;
		msz=SCENESZ;
		array=scene;
	}
	else
	{	
		l = listexist(ln);
		if (l<0) return;

		array=&listmem[lists[l]];
		sz = len[l];

		if (l==nol-1)
			msz=eol-lists[l];
		else
			msz=lists[l+1]-lists[l];
	}


	if (ind<0 || ind>msz-1)
		return; // error

	if (insrtflg)
	{
		int s2;
		if (ln=='!' || sz<msz) 
			s2=sz-1; 
		else 
			s2=sz-2;

		for (i=s2; i>=ind; i--)
			*(array+i+1)=*(array+i);

		if (ln=='!')
			nis++; 
		else if (sz<msz)
			len[l]++;
	}
	*(array+ind)=v;
}

// i.e. DELETE 5
// or   DELETE *
//      DELETE 5,7
//      DELETE 5,*
//      SELECT a,b
void listdelete(char ln, int indx1, int indx2, int selflg)
{
	int i,n2,l=0;
	int *array;
	int sz;
	
	if (ln=='!')
	{
		sz=nis;
		array=scene;
	}
	else
	{
		l = listexist(ln);
		if (l<0) return;

		array=&listmem[lists[l]];
		sz = len[l];
	}

	if (indx2==0) 
		indx2=sz-1;

	if (indx1<0 || indx1>indx2) 
		indx1=0;

	if (indx2<indx1 || indx2>sz) 
		indx2=sz-1;

	n2=indx2-indx1+1;

	switch(selflg) {
	case 1:
		{
			for (i=indx1; i<sz; i++)
				*(array+i)=*(array+i+n2);
			if (ln=='!')
				nis=nis-n2;
			else
				len[l]=len[l]-n2;
		}
		break;
	case 0:
		{
			// select
			for (i=0; i<n2; i++)
				*(array+i)=*(array+i+indx1);
			if (ln=='!')
				nis=n2;
			else
				len[l]=n2;
		}
		break;
	case 2:
		{
			if (ln=='!')
				nis=0;
			else
				len[l]=0;
		}
		break;
	}
}

extern char arrayname;

int list_eval(char ln, char *p, int ty)
{
	// eval list "5,1,2,3,4,5" 
	unsigned char r;
	int i,ind, sz;
	long n;
	char *t=p;

	if (dbg) {rprintf("Eval %c %d %s\n", ln, ty, p); }

	if (ty==1) 		// DATA
	{
		unsigned char *d=(unsigned char *)p;
		i = 0;
		if (*d != 0xFF) return 0;
		n = d[1];
		ind = listcreate(ln,n,1);
		for (i=0; i<n; i++)
			listwritei(ind, i, d[i+2]);
		return 0;
	}

	r = eval_expr(&t, &n);

	if (r==ARRAY)
	{

		//copy
		int l;
		if (ln==arrayname)
			return 0;

		if (arrayname=='!')
		{
			l = -1;
			sz=nis;
		}
		else
		{
			l = listexist(arrayname);
			if (l<0) return 0;
			sz=len[l];
		}

		if (ln=='!')
		{
			ind= -1;
			nis=sz;
		}
		else
			ind = listcreate(ln,sz,1);


if (dbg) {rprintf("Eval Array %c %d %d %d\n", arrayname,sz, ind, l); }

		for (i=0;i<sz;i++)
		{			
			listwritei(ind,i,listreadi(l,i));
		}
		return t-p;
	}
	
	if (r==STRING)
	{
		return 0;
	}

	if (ln != '!') 
	{
		ind = listcreate(ln,(int)n,1);
		sz  = len[ind];
		if (ind<0)
		{
			BasicErr=8; return 0;
		}
	}
	else
	{
		ind = -1;
		sz=nis=(int)n;
	}
	
	if (*t++ != ',') 
	{ 
		BasicErr=7; return 0; 
	}
	for (i=0;i<sz;i++)
	{
		n=0;
		eval_expr(&t, &n);
		if (i!=(sz-1) && *t++ != ',') 
		{ 
			BasicErr=6; return 0; 
		}
		listwritei(ind,i,(int)n);
	}
	return t-p;
}

void join(char ln1, char ln2)   // "@A . @B"
{
	int *arrayA, szA;
	int *arrayB, szB;
	int i;
	
	arrayA=listarray(ln1);
	szA   =listsize(ln1);
	arrayB=listarray(ln2);
	szB   =listsize(ln2);

	if (arrayA==0 || arrayB==0) return; //error
	
	if (szA+szB<SCENESZ) 
	{
		for (i=0;i<szA; i++)
		{
			scene[i] = arrayA[i];
		}
		for (i=0;i<szB; i++)
		{
			scene[i+szA] = arrayB[i];
		}
		nis=szA+szB;
	}
}

void add_sub(char ln1, char ln2, char op)   // "@A +/- @B"
{
	int *arrayA, szA;
	int *arrayB, szB;
	int i;
	
	arrayA=listarray(ln1);
	szA   =listsize(ln1);
	arrayB=listarray(ln2);
	szB   =listsize(ln2);

	if (arrayA==0 || arrayB==0) return; //error
	
	for (i=0;i<szA; i++)
	{
		if (i>=szB)
		{
			scene[i] = arrayA[i];
		}
		else
		{
		if (op=='+')
			scene[i] = arrayA[i] + arrayB[i];
		else
			scene[i] = arrayA[i] - arrayB[i];
		}
	}
	nis=szA;
}







