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

tOBJ emptyObj()
{
	tOBJ r;
	r.type=EMPTY;
	r.q=0;
	r.cnt=0;
	return r;
}

tOBJ emptyTObj(unsigned char t)
{
	tOBJ r = emptyObj();
	r.type=t;
        if (t==SYM)   r.string=NULL;
        if (t==CELL)  r.cell  =NULL;
        if (t==DICT)  r.dict  =NULL;
        if (t==STACK) r.stk   =NULL;
	if (t==FUNC)  r.fptr  =NULL;
	if (t==IMAG)  r.imgptr=NULL;
	if (t==FMAT2) r.fmat2 =NULL;
	if (t==RBM)   r.mot   =NULL;
	return r;
}

tOBJ emptyTPObj(unsigned char t, void *p)
{
	if (p==NULL) return emptyObj();
	tOBJ r = emptyTObj(t);
        if (t==SYM)   r.string=p;
        if (t==CELL)  r.cell  =p;
        if (t==DICT)  r.dict  =p;
        if (t==STACK) r.stk   =p;
	if (t==FUNC)  r.fptr  =p;
	if (t==IMAG)  r.imgptr=(oImage *)p;
	if (t==FMAT2) r.fmat2 =(fMatrix *)p;
	if (t==RBM)   r.mot   =(Motion *)p;
	return r;
}

tOBJ cloneObj(tOBJ z)
{
	tOBJ r = z; 
	switch (r.type)
	{
	case SYM: 	r.string = newstring(z.string);  	break;
	case CELL:
	case LAMBDA:	r.cell  = cloneCell((tCELLp)z.cell); 	break;
	case FMAT2:	r.fmat2 = fmatcp(z.fmat2); 		break;
	case STACK:	r.stk   =  clonestack(z.stk);  		break;		
	case DICT:	r.dict  =  clonedict(z.dict);  		break;		
	case RBM:	if (dbg) printf ("RBM clone TBD\n"); 	break;
	case IMAG:	r.imgptr =  cloneimage(z.imgptr);   	break;	
	default: 	/*if (dbg) printf ("nothing to clone\n");*/ break;
	}
	r.cnt=0;
	return r;
}

int freeobj(tOBJ *b)
{
	if (b==NULL) return 0;
	if (dbg) printf ("free obj - %d %s\n", b->cnt, objtype(*b));

	if (b->cnt>0)
	{
		if (dbg) printf ("copy not a clone\n");
		return 1;
	}

	switch (b->type)
	{
	case FMAT2: 	delmatrix(b->fmat2);  			break;
	case SYM: 	delstring(b->string); 			break;
	case CELL:
	case LAMBDA:	delCell(b->cell);     			break;
	case DICT:	deldict(b->dict);   			break;
	case RBM:	rbmdelete(b->mot); 			break;
	case STACK:	delstack(b->stk);			break;
	case IMAG:	delimage(b->imgptr);			break;	
	default: 	if (dbg) printf ("nothing to free\n"); 	break;
	}

	b->type=EMPTY;
	b->q=0;
	b->cnt=0;
	return 0;
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
	if (	r.type==CELL  || 
		r.type==FMAT2 || 
		r.type==STACK || 
		r.type==SYM   || 
		r.type==RBM   || 
		r.type==DICT  || 
		r.type==IMAG)
		r.cnt+=1;
	return r;
}

char *objtype(tOBJ t)
{
	char *st="??";
	switch (t.type)
	{  
	case INTGR: st="Int   "; break;
	case FLOAT: st="Float "; break;
	case FMAT2: st="Matrix"; break;
	case CELL:  st="List  "; break;
	case SYM:   st="Symbol"; break;
	case LAMBDA:st="Lambda"; break;
	case BOOLN: st="bool  "; break;
	case EMPTY: st="Empty "; break;
	case DICT:  st="Dict  "; break;
	case RBM:   st="RBM   "; break;
	case STACK: st="Stsck "; break;
	case FUNC:  st="Func  "; break;
	case IMAG:  st="Image "; break;
	}
	return st;
}

/**********************************************************/
/*  print                                                 */
/**********************************************************/

void printtype(FILE *fp, tOBJ r)
{
	switch (r.type)
	{
	case SYM: 	printstring (fp, r.string); 	break;
	case FMAT2:	fmatprint2  (r.fmat2);		break;
	case STACK:	stackprint2 (r.stk); 		break;		
	case DICT:	dict_print  (r.dict,0); 	break;		
	case IMAG:	printimage  (r.imgptr); 	break;	
	case INTGR: 	fprintf(fp, "%d", r.number);	break;
	case FLOAT: 	fprintf(fp, "%f", r.floatpoint);break;
	case RBM:	fprintf(fp, "RBM"); 		break;
	case LAMBDA:	fprintf(fp, "LAMBDA"); 		break;
	case FUNC:	fprintf(fp, "FUNCTION"); 	break;
	case EMPTY:     fprintf(fp, "NIL"); 		break;
	case BOOLN:     fprintf(fp, (r.number==0)?    "False":"True");  break;
	case CELL:	fprintf(fp, (r.cell != NULL)? "null" :"CELL");  break;
	default:	fprintf(fp, "? error - type\n"); 
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
        sp[n++]='(';
   	sp[n]  ='\0';

        n += sprint (sp+n, c->head);                
        while (c->tail != (void *)0)
        {
                c=c->tail;
        	sp[n++]=' ';
                n += sprint(sp+n,c->head);
        }
        sp[n++]=')';
   	sp[n]='\0';
	return n;
    }
    else if (r.type == INTGR)
    {
            sprintf(sp, "%d", r.number);

    }
    else if (r.type == EMPTY)
    {
            sprintf(sp, "NULL");

    }
    else if (r.type == FLOAT)
    {
            sprintf(sp, "%f", r.floatpoint);
    }
    else if (r.type == SYM)
    {
            sprintf(sp, "%s", r.string);
    }
    else
    {
	sprintf(sp+n,"%s", "X");
    }
    n=strlen(sp);
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
/*  conversions float   and integers                      */
/**********************************************************/


tOBJ makefloat(float f)
{	
	tOBJ r=emptyTObj(FLOAT);
	r.floatpoint=f;
	return r;
}

tOBJ makefloati(int i)
{	
	tOBJ r=emptyTObj(FLOAT);
	r.floatpoint=(float)i;
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
	tOBJ r=emptyTObj(INTGR);
	r.number=i;
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





