#ifndef OSTACK_H
#define OSTACK_H

#include "oobj.h"

typedef struct Stack {
	int  size;
	int  noe;
	tOBJ *objarray;
} tStack, *tStackp;

extern tOBJ makestack	(int n);
extern void delstack	(tStackp p);

extern int  push 	(tStackp p, tOBJ a);
extern tOBJ pop  	(tStackp p);
extern tOBJ peek 	(tStackp p, int n);
extern void clear	(tStackp p);
extern void stackprint	(tStackp p);
extern int  stacksize 	(tStackp p);

#endif
