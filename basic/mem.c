#include <stdlib.h>
#include <stdio.h>

#include "linux.h"

#include "oobj.h"
#include "ocells.h"
#include "ostring.h"
#include "odict.h"
#include "ofunction.h"
#include "oexpress.h"

//
// memory allocation for debug purposes
//
unsigned int mem_counter=0;

typedef struct mem {
	void *ptr;
	unsigned int size;
} Mem;

#define MAXLIST 2000
Mem memlist[MAXLIST];
int memlc=0;

int find(void *x)
{
	int i=0;
	while (i<memlc) {
		if (x==memlist[i].ptr) return i;
		i++;
	}
	return -1;
}

void *bas_malloc (size_t Size) 
{
	if (dbg) {printf ("malloc %d (%d)\n", Size,memlc);}
	void *p = malloc(Size);
	if (memlc<MAXLIST-1)
	{
		mem_counter += Size;
		memlist[memlc].size = Size;
		memlist[memlc].ptr  = p;
		memlc++;
	}
	return p;
}

void *bas_realloc(size_t Size, void *ptr)
{
	// tbd
	return realloc(Size, ptr);
}

void bas_free   (void *ptr)
{
	int n=find(ptr);
	if (dbg) {printf ("bas free - %d %u\n",n, (n>=0)?memlist[n].size:n);}
	if (n>=0) {
		mem_counter -= memlist[n].size;
		for (int i=n; i<memlc-1; i++) memlist[i]=memlist[i+1];
		memlc--;
	}
	free (ptr);
}

int bas_mem()
{
	if (dbg) {printf ("mem=%d - elements = %d\n", mem_counter, memlc);}
	return mem_counter;
}

void bas_show()
{
	printf("\n\n"); for (int i=0; i<memlc; i++) printf("%d) %d\n", i, memlist[i].size); printf("\n\n"); 
}

/**************************************************************************

Test harness for memory leeks

***************************************************************************/

void testme(int tn)
{
	tOBJ r,L,t;
	bas_show();

if (tn==0 || tn==1) 
{	printf ("\ntest 1\n\n");
	r = tokenise("");
	bas_show();
	t=r;
	L = read_from(&r);
	bas_show();
	freeobj(&t);
	bas_show();
	freeobj(&L);
	bas_show();
}

if (tn==0 || tn==2) 
{	printf ("\ntest 2\n\n");
	r = tokenise("test a token");
	bas_show();
	t=r;
	L = read_from(&r);
	bas_show();
	freeobj(&t);
	bas_show();
	freeobj(&L);
	bas_show();
}

if (tn==0 || tn==3) 
{	printf ("\ntest 3\n\n");
	r = tokenise("test ( this is ) a token");
	bas_show();
	t=r;
	L = read_from(&r);
	bas_show();
	freeobj(&t);
	bas_show();
	freeobj(&L);
	bas_show();	
}

if (tn==0 || tn==4) 
{	printf ("\ntest 4\n\n");
	r = parse("PR 'Hello 'hi");
	bas_show();
	freeobj(&r);
	bas_show();
}

if (tn==0 || tn==5) 
{	printf ("\ntest 5\n\n");
	r = parse("PR 'Hello 'Hi");
	bas_show();
	L = eval(r, env.dict);
	freeobj(&r);
	bas_show();
	freeobj(&L);
	bas_show();
}

if (tn==0 || tn==6) 
{	printf ("\ntest 6\n\n");
	r = parse("PR [1 2;3 4]");
	bas_show();
	L = eval(r, env.dict);
	freeobj(&r);
	bas_show();
	freeobj(&L);
	bas_show();
}

if (tn==0 || tn==7) 
{	printf ("\ntest 7\n\n");
	r = parse("DO (SETQ X 'Hello) (PR X)");
	bas_show();
	L = eval(r, env.dict);
	freeobj(&r);
	bas_show();
	freeobj(&L);
	bas_show();
}

if (tn==0 || tn==8) 
{	//printf ("\ntest 8-a\n\n");
	//r = tokenise("DO (SETQ X 'Hi-there) (PR X) (SETQ X 'Hello)");
	//bas_show();
	//t=r;
	//L = read_from(&r);
	//bas_show();
	//freeobj(&t);
	//bas_show();
	//freeobj(&L);
	//bas_show();

	printf ("\ntest 8\n\n");
	r = parse("DO (SETQ X 'Hi-there) (SETQ X 1.5) (SETQ X 'Hello)");
	bas_show();
	L = eval(r, env.dict);
	freeobj(&r);
	bas_show();
	freeobj(&L);
	bas_show();
}

if (tn==0 || tn==9) 
{	printf ("\ntest 9\n\n");
	r = parse("PR [1 2; 3 4]");
	bas_show();
	L = eval(r, env.dict);
	freeobj(&r);
	bas_show();
	freeobj(&L);
	bas_show();
}

if (tn==0 || tn==10) 
{	printf ("\ntest 10\n\n");
	r = parse("DO (SETQ S (STACK 5)) (PUSH S 'Hello) (POP S) ");
	bas_show();
	L = eval(r, env.dict);
	freeobj(&r);
	bas_show();
	freeobj(&L);
	bas_show();
}

if (tn==0 || tn==11) 
{	printf ("\ntest 11\n\n");
	r = parse("DO (SETQ ENV (DICT '((AZ 1.0) (BZ 2.0)))) (PR ENV) (SETK '(ENV AZ 'Hello))");
	bas_show();
	L = eval(r, env.dict);
	freeobj(&r);
	bas_show();
	freeobj(&L);
	bas_show();
}

printf ("\nDONE\n\n");

}




