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

#include "ostack.h"
#include "mem.h"


/**********************************************************/
/*  stack                                                 */
/**********************************************************/

tOBJ makestack(int n, unsigned char t)
{
	tStackp  p;
	if (dbg) printf ("Make stack %d\n", n);

	p=(tStackp)bas_malloc(sizeof(tStack));
	p->size=n;
	p->noe=0;
	p->type=t; 
	p->objarray = bas_malloc(sizeof(tOBJ)*n);
	return emptyTPObj(STACK, p);
}

void delstack(tStackp p)
{
	if (dbg) printf ("Del stack\n");
	if (p != NULL)
	{
		bas_free(p->objarray);
		bas_free(p);
		p=0;
	}
}

tStackp clonestack(tStackp x)
{
	int i;
	tStackp  p=(tStackp)bas_malloc(sizeof(tStack));
	p->size=x->size;
	p->noe=x->noe;
	p->type=x->type;
	p->objarray = bas_malloc(sizeof(tOBJ)*p->size);

	for(i=0; i<p->noe; i++)
	{
		p->objarray[i] =cloneObj(x->objarray[i]	);
	}

	return p;
}

int push(tStackp st, tOBJ a)
{
	if (dbg)  { printf ("PUSH %d = ", a.type); print(a); printf("\n"); }

	if (st->noe < st->size)
	{
		st->objarray[st->noe++] = cloneObj(a);
		return 1;
	}
	return 0;
}

tOBJ pop(tStackp st)
{
	tOBJ e=emptyObj();

	if (st->noe>0 && st->type==0)
	{
		e=st->objarray[st->noe-1];
		st->noe--;
	}

	if (st->noe>0 && st->type==1)
	{
		int i;
		e=st->objarray[0];
		for (i=1; i<st->noe; i++)
			st->objarray[i-1]=st->objarray[i];
		st->noe--;
	}

	if (dbg) { printf ("POP %d = ", e.type);  print(e); printf("\n"); }

	return e; //cloneObj(e);
}

tOBJ peek(tStackp st, int n)
{
	tOBJ e;
	e.type=EMPTY;

	if (st->noe-1-n >= 0 && st->type==0)
	{
		return  cloneObj(st->objarray[st->noe-1-n]);
	}

	if (st->noe-1-n >= 0 && st->type==1)
	{
		return  cloneObj(st->objarray[n]);
	}
	return e;
}

void clear(tStackp st)
{
	st->noe=0;
}

void stackprint(tStackp st)
{
	int i;
	printf ("Stack\n");
	for (i=0;i<st->noe;i++)
	{
		rprintf ("%d ", i);
		print(st->objarray[i]);
		rprintfCRLF();
	}
}

void stackprint2(tStackp st)
{
	printf ("Stack %d / %d\n", st->noe, st->size);
}

int stacksize(tStackp st)
{
	return st->noe;
}

