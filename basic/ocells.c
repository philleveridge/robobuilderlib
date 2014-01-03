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

extern int dbg;

/**********************************************************/
/*  CELLS                                                 */
/**********************************************************/


tCELLp newCell()
{
	if (dbg) printf ("New CELL\n");	
	return (tCELLp)malloc(sizeof(tCELL));
}

void delCell(tCELLp p)
{
	if (dbg)  printf ("Delete CELL\n");	
	freeobj(&p->head);
	if (p->tail !=NULL) delCell((tCELLp)p->tail);
	free(p);
}

tOBJ makeCell()
{
	tOBJ r; tCELLp p;
	r.type=CELL;
	p = newCell();
	p->head=emptyObj();
	p->tail=0;
	r.cell=p;
	r.cnt=0;
	return r;
}

tOBJ makeCell2(tOBJ a, tCELLp b)
{
	// a + {b} => {a b}
	tOBJ r; tCELLp p;
	r.type=CELL;
	p = newCell();
	p->head=cloneObj(a);  //clone ?
	p->tail=b;
	r.cell=p;
	r.cnt=0;
	return r;
}

tOBJ makeCell3(tOBJ a, tCELLp b)
{
	// a + {b} => {b a}
	tOBJ r; tCELLp p;
	r.type=CELL;
	p = newCell();
	p->head=cloneObj(a);  //clone ?
	p->tail=NULL;

	while (b->tail != NULL) b=b->tail;
	r.cell=b;
	return r;
}

tCELLp cloneCell(tCELLp p)
{
	tCELLp top, t,r=newCell();
	top=r;
	while (p!=NULL)
	{
		r->head =cloneObj(p->head);  //cloneObj??
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





