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

extern int dbg;

/**********************************************************/
/*  stack                                                 */
/**********************************************************/

tOBJ makestack(int n)
{
	tOBJ r;
	tStackp  p=(tStackp)bas_malloc(sizeof(tStack));
	p->size=n;
	p->noe=0;
	p->objarray = bas_malloc(sizeof(tOBJ)*n);

	if (dbg) printf ("Make stack %d\n", n);

	r.type=STACK;
	r.stk=(void *)p;
	return r;
}

void delstack(tStackp p)
{
	printf ("Del stack\n");
	if (p != NULL)
	{
		free(p->objarray);
		free(p);
		p=0;
	}
}

tStackp clonestack(tStackp x)
{
	tStackp  p=(tStackp)bas_malloc(sizeof(tStack));
	p->size=x->size;
	p->noe=x->noe;
	p->objarray = bas_malloc(sizeof(tOBJ)*p->size);

	for(int i=0; i<p->noe; i++)
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
	tOBJ e;
	e.type=EMPTY;

	if (st->noe>0)
	{
		e=st->objarray[st->noe-1];
		st->noe--;
	}

	if (dbg) { printf ("POP %d = ", e.type);  print(e); printf("\n"); }

	return cloneObj(e);
}

tOBJ peek(tStackp st, int n)
{
	tOBJ e;
	e.type=EMPTY;

	if (st->noe-1-n >= 0)
	{
		return  cloneObj(st->objarray[st->noe-1-n]);
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
	int i;
	printf ("Stack %d / %d\n", st->noe, st->size);
}

int stacksize(tStackp st)
{
	return st->noe;
}

