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
	if (r.type==CELL || r.type==FMAT2 || r.type==STACK || r.type==SYM  || r.type==RBM   || r.type==DICT  || r.type==IMAG)
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
	else if (r.type==IMAG)
	{
		r.imgptr =  cloneimage(z.imgptr);  	
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
    else  if (r.type == IMAG)
    {
	printimage(r.imgptr);
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
/*  conversuins float   and integers                      */
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
	case BOOLN: st="bool  "; break;
	case EMPTY: st="Empty "; break;
	case DICT:  st="Dict  "; break;
	case RBM:   st="RBM   "; break;
	case STACK: st="STACK "; break;
	case FUNC:  st="Func  "; break;
	case IMAG:  st="Image "; break;
	}
	return st;
}




