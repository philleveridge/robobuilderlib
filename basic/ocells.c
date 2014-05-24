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

#include "ocells.h"
#include "mem.h"
#include "ofunction.h"

/**********************************************************/
/*  CELLS                                                 */
/**********************************************************/


tCELLp newCell()
{
	if (dbg) printf ("New CELL\n");	
	return (tCELLp)bas_malloc(sizeof(tCELL));
}

void delCell(tCELLp p)
{
	if (p==NULL) return;
	if (dbg)  println ("Delete CELL -", p->head);	
	freeobj(&p->head);
	if (p->tail !=NULL) delCell((tCELLp)p->tail);
	if ( p!=NULL) bas_free(p);
	
}

tOBJ makeCell()
{
	tOBJ r = emptyTPObj(CELL, newCell());
	tCELLp p = r.cell;
	if (p !=NULL)
	{
		p->head=emptyObj();
		p->tail=0;
	}
	return r;
}

tOBJ makeCell2(tOBJ a, tCELLp b)
{
	// a + {b} => {a b}
	tOBJ r = emptyTPObj(CELL, newCell());
	tCELLp p = r.cell;
	if (p !=NULL)
	{
		p->head=cloneObj(a); 
		p->tail=b;
	}
	return r;
}

tCELLp cloneCell(tCELLp p)
{
	tCELLp top, t,r=newCell();
	top=r;
	while (p!=NULL)
	{
		r->head =cloneObj(p->head); 
		p=p->tail;
		t=r;
		if (p!=NULL)
		{
			r=newCell();
			t->tail=r;
		}
		else
			t->tail=NULL;
	}
		
	return top;
}

tOBJ append(tOBJ a, tOBJ b)
{
	// {} A  -> {A}
	// {B} A -> (B A}
	// B A   -> NIL
	tOBJ r;	
	tCELLp m,n,p;

	if (a.type == EMPTY) 
	{
		r=makeCell();
		((tCELLp)(r.cell))->head = cloneObj(b);
		((tCELLp)(r.cell))->tail = NULL;
		return r;
	}

	if (a.type != CELL)
		return emptyObj();
	
	r=cloneObj(a);
	
	// walk to end
	n=NULL;
	p = r.cell;

	while (p!= NULL)
	{
		n=p;
		p=p->tail;
	}

	// add 'b'
	m = newCell();
	n->tail=m;
	m->head = cloneObj(b);
	m->tail = NULL;

	return r;
}

/**********************************************************/
/*  conversions Arrays to List  (and back)                */
/**********************************************************/

tOBJ cnvtInttoList(int an, int *array)
{
	int i;
	tOBJ r=emptyObj();
	if (an<=0) return r;

	for (i=an-1; i>=0; i--)
	{
		tOBJ t=ocons(makeint((int)array[i]), r);
		freeobj(&r);
		r=t;
	}
	return r;
}

tOBJ cnvtBytetoList(int an, BYTE *array)
{
	int i;
	tOBJ r=emptyObj();
	if (an<=0) return r;

	for (i=an-1; i>=0; i--)
	{
		tOBJ t=ocons(makeint(abs((int)array[i])%256), r);
		freeobj(&r);
		r=t;
	}
	return r;
}

tOBJ cnvtFloattoList(int an, float *array)
{
	int i;
	tOBJ r=emptyObj();
	if (an<=0) return r;

	for (i=an-1; i>=0; i--)
	{
		tOBJ t=ocons(makefloat((float)array[i]), r);
		freeobj(&r);
		r=t;
	}
	return r;
}

int cnvtListtoByte(tOBJ lst, int an, BYTE *array)
{
	int cnt=0;
	if (lst.type != CELL) return 0;
	while (cnt<an && onull(lst).number==0)
	{
		array[cnt++] = abs(toint(ocar(lst)))%256;
		lst=ocdr(lst); 
	}
	return cnt;
}

int cnvtListtoInt(tOBJ lst, int an, int *array)
{
	int cnt=0;
	if (lst.type != CELL) return 0;
	while (cnt<an && onull(lst).number==0)
	{
		array[cnt++] = toint(ocar(lst));
		lst=ocdr(lst); 
	}
	return cnt;
}

int cnvtListtoFloat(tOBJ lst, int an, float *array)
{
	int cnt=0;
	if (lst.type != CELL) return 0;
	while (cnt<an && onull(lst).number==0)
	{
		array[cnt++] = tofloat(ocar(lst));
		lst=ocdr(lst); 
	}
	return cnt;
}





