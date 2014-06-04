#include <stdlib.h>
#include <stdio.h>

#include "linux.h"

#include "oobj.h"
#include "ocells.h"
#include "ostring.h"
#include "odict.h"
#include "ofunction.h"
#include "oexpress.h"

#ifdef MEM_DEBUG

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


void test_extend(char *s, char *expected, int nob)
{
	char buffer[100];
	init_extend();
	int tc=memlc;
	*buffer=0;

	dbg=0;
	tOBJ e=parse(s);
	dbg=1;

	tOBJ v = eval(e, env.dict);
	println (" = ", v);
	
	sprint(buffer, v);

	printf("-- free return value\n"); 
	freeobj(&v);
	printf("-- free parse output\n");
	freeobj(&e);

	printf("-- used %d\n", memlc-tc);
	if (nob != (memlc-tc))
	{
		printf("-- ERROR [ %d ] != %d\n", memlc-tc, nob);
	}

	if (!(expected==NULL || *expected==0))
	{
		if (strcmp(buffer,expected))
		{
			printf("-- ERROR [ %s ] != [ %s ]\n", expected, buffer);
		}
	}

}


#else

int memlc=0;

void *bas_malloc (size_t Size) 
{
	void *p;
	if (dbg) {printf ("malloc\n"); fflush(stdout);}
	p = malloc(Size);
	return p;
}

void *bas_realloc(size_t Size, void *ptr)
{
	// tbd
	return realloc(Size, ptr);
}

void bas_free   (void *ptr)
{
	if (dbg) {printf ("bas free \n"); fflush(stdout);}
	free (ptr);
}

int bas_mem()
{
	return 0;
}

void bas_show()
{
}

void test_extend(char *s, char *expected, int nob)
{
	tOBJ e,v;
	char buffer[100];
	init_extend();
	*buffer=0;

	dbg=0;
	e=parse(s);
	dbg=1;

	v = eval(e, env.dict);
	println (" = ", v);
	
	sprint(buffer, v);

	printf("-- free return value\n"); 
	freeobj(&v);
	printf("-- free parse output\n");
	freeobj(&e);

	if (!(expected==NULL || *expected==0))
	{
		if (strcmp(buffer,expected))
		{
			printf("-- ERROR [ %s ] != [ %s ]\n", expected, buffer);
		}
	}

}

#endif


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
	test_extend("PR 'Hello 'hi", "hi", 0);
	testfooter();
}

if (tn==0 || tn==5) 
{		
	testheader(51);
	test_extend("PR (SIN 0.5)", "0.479426", 0);
	testfooter();

	testheader(52);
	test_extend("PR '(1 2 3)", "(1 2 3)", 0);
	testfooter();

	testheader(53);
	test_extend("CAR '(1 2 3)", "1", 0);
	testfooter();

	testheader(54);
	test_extend("CDR '(1 2 3)", "(2 3)", 0);
	testfooter();
}

if (tn==0 || tn==6) 
{	
	testheader(6);
	test_extend("PR [1 2;3 4]", "", 0);
	testfooter();
}

if (tn==0 || tn==7) 
{	
	//testheader(7);
	//test_extend("DO (SETQ X 'Hello)");
	//testfooter();

	//testheader(71);
	//test_extend("DO (SETQ Y 'Hi-there) (SETQ Y 1.5) (SETQ Y 'Hello)");
	//testfooter();

	testheader(72);
	test_extend("SETQ Z '(1 2 3)", "(1 2 3)", 4);
	testfooter();

	testheader(73);
	test_extend("CDR Z", "(2 3)", 0);
	testfooter();
}

if (tn==0 || tn==8) 
{	
	testheader(8);
	test_extend("FOR X '(1 8) (PR X)", "", 1);
	testfooter();
}

if (tn==0 || tn==9) 
{	
	testheader(9);
	test_extend("FUNC FOO (X) (+ X 1))", "", 10);
	testfooter();

	testheader(91);
	test_extend("FOO 5", "6", 0);
	testfooter();

	testheader(92);
	test_extend("FOO (FOO 5)", "7", 0);
	testfooter();
}

if (tn==0 || tn==10) 
{

	testheader(10);
	test_extend("STACK 5", "", 0);
	testfooter();

	testheader(101);
	test_extend("SETQ S (STACK 5)", "", 3);
	testfooter();
	
	testheader(102);
	test_extend("PUSH S 'Hello", "Hello", 1);
	testfooter();

	testheader(103);
	test_extend("POP S", "Hello", -1);
	testfooter();
}

if (tn==0 || tn==11) 
{
	testheader(11);
	test_extend("SETQ ENV (DICT '((AZ 1.0) (BZ 2.0)))", "", 5);
	testfooter();
	
	testheader(111);
	test_extend("PR ENV", "", 0);
	testfooter();

	testheader(112);
	test_extend("SETK 'ENV 'AZ 'Hello", "", 1);
	testfooter();

	testheader(113);
	test_extend("GETK 'ENV 'AZ", "Hello", 0);
	testfooter();
}

if (tn==0 || tn==12) 
{

	testheader(12);	
	test_extend("FUNC TST (X) (CAR X)", "", 9);
	testfooter();

	testheader(121);		
	test_extend("TST '(1 2 3)", "1", 0);
	testfooter();

	testheader(122);		
	test_extend("SETQ S '(1 2 3)", "(1 2 3)", 1);
	testfooter();

	testheader(123);		
	test_extend("TST S", "1", 0);
	testfooter();
}



if (tn==0 || tn==13) 
{

	testheader(13);	
	test_extend("FUNC ROUND (X) (INT (+ 0.5 X))", "", 13);
	testfooter();

	testheader(131);		
	test_extend("DEF CHT  (X Y T) (AND (= X (ROUND (CAR T))) (= Y (ROUND (CAR (CDR T)))))", "", 40);
	testfooter();

	testheader(132);		
	test_extend("CHT 1 2 '(1.1 2.1 0.1)", "", 0);
	testfooter();

	testheader(133);		
	test_extend("CHT 1 2 '(1.8 2.1 0.1)", "", 0);
	testfooter();

}

if (tn==0 || tn==14) 
{

	testheader(14);	
	test_extend("LIST 1.0 2.0 3.0", "(1.000000 2.000000 3.000000)", 0);
	testfooter();
}

if (tn==0 || tn==15) 
{

	testheader(15);	
	test_extend("CONS 1 '(1 2 3)", "(1 1 2 3)", 0);
	testfooter();
}

if (tn==0 || tn==16) 
{
	testheader(16);	
	test_extend("DEF BAR (X) (PR X)", "", 9);
	testfooter();

	testheader(161);	
	test_extend("BAR '(1 2 3)", "(1 2 3)", 0);
	testfooter();
}

if (tn==0 || tn==17) 
{
	testheader(17);	
	test_extend("CAR '( (AB 1) (AC 2) )", "(AB 1)", 0);
	testfooter();

	testheader(171);	
	test_extend("CAR '( (AB 1) (AC 2) )", "(AB 1)", 0);
	testfooter();
}

if (tn==0 || tn==18) 
{
	testheader(18);	
	test_extend("COND 'A 'B 'C", "B", 0);
	testfooter();

	testheader(181);	
	test_extend("COND 1 'B 'C", "B", 0);
	testfooter();

	testheader(182);	
	test_extend("COND 0 'A 0 'B 1 'E", "E", 0);
	testfooter();
}

if (tn==0 || tn==19) 
{
	testheader(19);	
	test_extend("PLUS 'ABC 'DE", "ABCDE", 0);
	testfooter();
}

if (tn==0 || tn==20) 
{

	testheader(20);	
	test_extend("MAPCAR SIN '(0.100000 0.200000 0.300000)", "", 0);
	testfooter();

	testheader(201);	
	test_extend("MAPCAR (LAMBDA (X) (+ X 1)) '(1 2 3 4)", "(2 3 4 5)", 0);
	testfooter();

	testheader(202);	
	test_extend("MAPCAR SIN [0.1 0.2 ; 0.3 0.4 ]", "", 0);
	testfooter();
}

printf ("\nDONE\n\n");

}

