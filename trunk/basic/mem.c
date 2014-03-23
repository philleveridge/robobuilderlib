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
	if (dbg) {printf ("malloc %d (%d)\n", Size,memlc); fflush(stdout);}
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
	if (dbg) {printf ("bas free - %d %u\n",n, (n>=0)?memlist[n].size:n); 	fflush(stdout);}
	if (n>=0) {
		mem_counter -= memlist[n].size;
		for (int i=n; i<memlc-1; i++) memlist[i]=memlist[i+1];
		memlc--;
	}
	free (ptr);
}

int bas_mem()
{
	if (dbg) {printf ("mem=%d - elements = %d\n", mem_counter, memlc); 	fflush(stdout);}
	return mem_counter;
}

void bas_show()
{
	printf("\n\n"); for (int i=0; i<memlc; i++) printf("%d) %d\n", i, memlist[i].size); printf("\n\n"); 
	fflush(stdout);
}

/**************************************************************************

Test harness for memory leeks

***************************************************************************/
	
int tot;

void testheader(int n)
{
	printf ("\n--------------------------------------------------------------------------\ntest %d\n\n",n);
	bas_show();
	fflush(stdout);
}

void testfooter()
{	bas_show(); 
	printf ("\nUSED MEM LOC  %d\n", memlc-tot); tot=memlc;
	fflush(stdout);
}

void testme(int tn)
{

	tOBJ r,L,t;
	bas_show(); 
	tot=memlc;

if (tn==0 || tn==1) 
{	
	testheader(1);
	r = tokenise("");
	bas_show();
	t=r;
	L = read_from(&r);
	bas_show();
	freeobj(&t);
	bas_show();
	freeobj(&L);
	testfooter();
}

if (tn==0 || tn==2) 
{		
	testheader(2);
	r = tokenise("test a token");
	bas_show();
	t=r;
	L = read_from(&r);
	bas_show();
	freeobj(&t);
	bas_show();
	freeobj(&L);
	testfooter();
}

if (tn==0 || tn==3) 
{		
	testheader(3);
	r = tokenise("test ( this is ) a token");
	bas_show();
	t=r;
	L = read_from(&r);
	bas_show();
	freeobj(&t);
	bas_show();
	freeobj(&L);
	testfooter();
}

if (tn==0 || tn==4) 
{		
	testheader(4);
	extend("PR 'Hello 'hi");
	testfooter();
}

if (tn==0 || tn==5) 
{		
	testheader(51);
	extend("PR (SIN 0.5)");
	testfooter();

	testheader(52);
	extend("PR '(1 2 3)");
	testfooter();

	testheader(53);
	extend("CAR '(1 2 3)");
	testfooter();

	testheader(54);
	extend("CDR '(1 2 3)");
	testfooter();
}

if (tn==0 || tn==6) 
{	
	testheader(6);
	extend("PR [1 2;3 4]");
	testfooter();
}

if (tn==0 || tn==7) 
{	
	//testheader(7);
	//extend("DO (SETQ X 'Hello)");
	//testfooter();

	//testheader(71);
	//extend("DO (SETQ Y 'Hi-there) (SETQ Y 1.5) (SETQ Y 'Hello)");
	//testfooter();

	testheader(72);
	extend("SETQ Z '(1 2 3)");
	testfooter();

	testheader(73);
	extend("CDR Z");
	testfooter();
}

if (tn==0 || tn==8) 
{	
	testheader(8);
	extend("FOR X '(1 8) (PR X)");
	testfooter();
}

if (tn==0 || tn==9) 
{	
	testheader(9);
	extend("FUNC FOO (X) (+ X 1))");
	testfooter();

	testheader(91);
	extend("FOO 5");
	testfooter();

	testheader(92);
	extend("FOO (FOO 5)");
	testfooter();
}

if (tn==0 || tn==10) 
{

	testheader(10);
	extend("STACK 5");
	testfooter();

	testheader(101);
	extend("SETQ S (STACK 5)");
	testfooter();
	
	testheader(102);
	extend("PUSH S 'Hello");
	testfooter();

	testheader(103);
	extend("POP S");
	testfooter();
}

if (tn==0 || tn==11) 
{
	testheader(11);
	extend("SETQ ENV (DICT '((AZ 1.0) (BZ 2.0)))");
	testfooter();
	
	testheader(111);
	extend("PR ENV");
	testfooter();

	testheader(112);
	extend("SETK '(ENV AZ 'Hello)");
	testfooter();

	testheader(113);
	extend("GETK ENV AZ");
	testfooter();
}

if (tn==0 || tn==12) 
{

	testheader(12);	
	extend("FUNC TST (X) (CAR X)");
	testfooter();

	testheader(121);		
	extend("TST '(1 2 3)");
	testfooter();

	testheader(122);		
	extend("SETQ S '(1 2 3)");
	testfooter();

	testheader(123);		
	extend("TST S");
	testfooter();
}

if (tn==0 || tn==13) 
{

	testheader(13);	
	extend("FUNC ROUND (X) (INT (+ 0.5 X))");
	testfooter();

	testheader(131);		
	extend("DEF CHT  (X Y T) (AND (= X (ROUND (CAR T))) (= Y (ROUND (CAR (CDR T)))))");
	testfooter();

	testheader(132);		
	extend("CHT 1 2 '(1.1 2.1 0.1)");
	testfooter();

	testheader(133);		
	extend("CHT 1 2 '(1.8 2.1 0.1)");
	testfooter();

}
printf ("\nDONE\n\n");

}




