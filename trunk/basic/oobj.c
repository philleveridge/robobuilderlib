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

#include "oobj.h"
#include "ocells.h"
#include "ostring.h"
#include "odict.h"
#include "rbmread.h"
#include "ofunction.h"
#include "ostack.h"

/**********************************************************/
/* Objects                                                */
/**********************************************************/

int freeobj(tOBJ *b)
{
	if (b==NULL) return;
	if (dbg) printf ("free obj - %d %s\n", b->cnt, objtype(*b));

	if (b->cnt>0)
	{
		if (dbg) printf ("copy not a clone\n");
		return 1;
	}

	if (b->type==FMAT2)
	{
		if (dbg) printf ("free mat\n");
		if (b->cnt==0) delmatrix(b->fmat2);
	}
	else if (b->type==SYM)
	{
		if (dbg) printf ("free SYM %s\n",b->string);
		if (b->cnt==0) delstring(b->string);
	}
	else if (b->type==CELL || b->type==LAMBDA  )
	{
		if (dbg) printf ("free cell\n");
		if (b->cnt==0) delCell(b->cell);
	}
	else if (b->type==DICT)
	{
		if (dbg) printf ("free dict\n");
		if (b->cnt==0) deldict(b->dict);
	}
	else if (b->type==RBM)
	{
		if (b->cnt==0) rbmdelete(b->mot);
	}
	else if (b->type==STACK)
	{
		if (b->cnt==0) delstack(b->stk);
	}	
	else 
	{
		if (dbg) printf ("nothing to free\n");
	}
	b->type=EMPTY;
	b->q=0;
	b->cnt=0;
	return 0;
}

tOBJ emptyObj()
{
	tOBJ r;
	r.type=EMPTY;
	r.q=0;
	r.cnt=0;
	return r;
}

int compareObj(tOBJ a, tOBJ b)
{
	if (a.type != b.type) return 0; //false;

	//SYM, INTGR, BOOLN, FUNC, FLOAT, STR, CELL, EMPTY, FMAT2};
	if (a.type == INTGR) return a.number==b.number;
	if (a.type == FLOAT) return a.floatpoint==b.floatpoint;
	if (a.type == SYM )   return !strcmp(a.string,b.string);
	if (a.type == EMPTY) return 1;

	if (a.type == CELL)
	{
		while (a.type != EMPTY)
		{
			if (!compareObj(ocar(a), ocar(b))) return 0;
			a=ocdr(a); b=ocdr(b);
		}
		return 1;
	}
	return 0;
}

tOBJ copyObj(tOBJ z)
{
	tOBJ r = z; 
	if (r.type==CELL || r.type==FMAT2 || r.type==STACK || r.type==SYM  || r.type==RBM   || r.type==DICT)
		r.cnt+=1;
	return r;
}

tOBJ cloneObj(tOBJ z)
{
	tOBJ r = z; 

	if (r.type==SYM)
	{
		r.string = newstring(z.string);
	}
	else if (r.type==CELL || r.type==LAMBDA)
	{
		r.cell = cloneCell((tCELLp)z.cell);
	}
	else if (r.type==FMAT2)
	{
		r.fmat2 = fmatcp(z.fmat2);
	}
	else if (r.type==STACK)
	{
		//if (dbg) printf ("clone STACK\n");
		r.stk =  clonestack(z.stk); 		
	}
	else if (r.type==DICT)
	{
		r.dict =  clonedict(z.dict); 		
	}
	else if (r.type==RBM)
	{
		if (dbg) printf ("RBM clone TBD\n");
	}
	else 
	{
		//if (dbg) printf ("nothing to clone\n");
	}

	r.cnt=0;
	return r;
}

/**********************************************************/
/*  print                                                 */
/**********************************************************/

void printtype(FILE *fp, tOBJ r)
{
    if (r.type == INTGR)
    {
            fprintf(fp, "%d", r.number);
    }
    else if (r.type == FLOAT)
    {
            fprintf(fp, "%f", r.floatpoint);
    }
    else if (r.type == SYM)
    {
		char ch, *cp=r.string;
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
    else if (r.type == BOOLN)
    {
        if ( r.number==0)
                fprintf(fp, "False");
        else
                fprintf(fp, "True");
    }
    else if (r.type == EMPTY)
    {
         fprintf(fp, "NIL");
    }
    else if (r.type == LAMBDA)
    {
         fprintf(fp, "LAMBDA");   
    }
    else if (r.type == FUNC)
    {
         fprintf(fp, "FUNCTION");   
    }
    else if (r.type == RBM)
    {
         fprintf(fp, "RBM\n");  
	 rbmprint (r.mot);
    }
    else if (r.type == FMAT2)
    {
	fmatprint2(r.fmat2);	 
    }
    else  if (r.type == DICT)
    {
	dict_print(r.dict,0);
    }
    else  if (r.type == STACK)
    {
	stackprint2(r.stk);
    }
    else  if (r.type == CELL)
    {
	if (r.cell != NULL)
	{
		 fprintf(fp, "CELL");	
	}
	else
	{
		 fprintf(fp, "null");
	}
    }  
    else
    {               
         fprintf(fp, "? error - type\n");   
    }
    return;
}

tOBJ fprint(FILE *fp, tOBJ r)
{
    if (r.q==1) fprintf (fp,"'");

    if (r.type == CELL)
    {
        struct cell  *c = r.cell;
        fprintf(fp,"(");  
        fprint (fp, c->head);
                
        while (c->tail != (void *)0)
        {
                c=c->tail;
                fprintf(fp, " ");
                fprint(fp,c->head);
        }
        fprintf(fp,")");
    }
    else
    {
		printtype(fp, r);
    }
    return r;
}

int sprint(char *sp, tOBJ r)
{
    int n=0;
    if (r.q==1) sp[n++]='\'';

    if (r.type == CELL)
    {
        struct cell  *c = r.cell;
        sp[n++]=='(';
        n += sprint (sp, c->head);
                
        while (c->tail != (void *)0)
        {
                c=c->tail;
        	sp[n++]=' ';
                n += sprint(sp,c->head);
        }
        sp[n++]=')';
    }
    else if (r.type == INTGR)
    {
            sprintf(sp, "%d", r.number);
    }
    else if (r.type == FLOAT)
    {
            sprintf(sp, "%f", r.floatpoint);
    }
    else if (r.type == SYM)
    {
            sprintf(sp, "%S", r.string);
    }
    else
    {
	sprintf(sp,"%s", "X");
    }
    sp[n]='\0';
    return n;
}

tOBJ print(tOBJ r)
{
	return fprint(stdout, r);
}

tOBJ println(char *s, tOBJ r)
{
	printf ("%s", s); print(r); printf("\n");
	return r;
}

void pp(tOBJ x, int n)
{
	int i;
	while (x.type != EMPTY)
	{
		tOBJ e = ocar(x);
		x=ocdr(x);
		for (i=0;i<n; i++) printf (" "); print(e); printf("\n");
		if (e.type==CELL)
			pp(e,n+2);
	}
}


/**********************************************************/
/*  conversuins float   and integers                      */
/**********************************************************/


tOBJ makefloat(float f)
{	
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=f;
	r.q=0;
	r.cnt=0;
	return r;
}

tOBJ makefloati(int i)
{	
	tOBJ r;
	r.type=FLOAT;
	r.floatpoint=(float)i;
	r.q=0;
	r.cnt=0;
	return r;
}

float tofloat(tOBJ v)
{
	float f=0.0;
	if (v.type==FLOAT)
		f=v.floatpoint;
	else
	if (v.type==INTGR)
		f=(float)v.number;

	return f;
}

tOBJ makeint(int i)
{	
	tOBJ r;
	r.type=INTGR;
	r.number=i;
	r.q=0;
	r.cnt=0;
	return r;
}

int toint(tOBJ v)
{
	int f=0;
	if (v.type==FLOAT)
		f=(int)v.floatpoint;
	else
	if (v.type==INTGR)
		f=v.number;
	return f;
}

tOBJ cnvtInttoList(int an, int *array)
{
	tOBJ r,n,top;
	int i;

	if (an<=0) return emptyObj();
	top=makeCell2( makeint(array[0]), NULL);
	r=top;

	for (i=1; i<an; i++)
	{
		n=makeCell2(makeint(array[i]), NULL);
		((tCELLp)(r.cell))->tail = n.cell;
		r=n;
	}
	((tCELLp)(n.cell))->tail = 0;
	return top;
}

tOBJ cnvtBytetoList(int an, BYTE *array)
{
	int i;
	tOBJ r=emptyObj();
	if (an<=0) return r;

	for (i=an-1; i>=0; i--)
	{
		r=ocons(makeint((int)array[i]), r);
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
		r=ocons(makefloat((int)array[i]), r);
	}

	return r;
}


int cnvtListtoByte(tOBJ lst, int an, BYTE *array)
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

char *objtype(tOBJ t)
{
	char *st;
	switch (t.type)
	{  
	case INTGR: st="Int   "; break;
	case FLOAT: st="Float "; break;
	case FMAT2: st="Matrix"; break;
	case CELL:  st="List  "; break;
	case SYM:   st="Symbol"; break;
	case LAMBDA:st="Lambda"; break;
	case BOOLN: st="bool  ";break;
	case EMPTY: st="Empty ";break;
	case DICT:  st="Dict  ";break;
	case RBM:   st="RBM   ";break;
	case STACK: st="STACK ";break;
	case FUNC:  st="Func  ";break;
	}
	return st;
}




