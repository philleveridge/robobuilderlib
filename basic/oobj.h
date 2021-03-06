#ifndef OOBJ_H
#define OOBJ_H

#ifdef WIN32
#include "win.h"
#endif

#include "fmatrix.h"
#include "rbmread.h"
#include "oimg.h"
#include "oparticles.h"

enum  TYPE  {SYM, INTGR, BOOLN, FUNC, FLOAT, CELL, EMPTY, FMAT2, DICT, LAMBDA, RBM, STACK, IMAG , TURTLE, PARTICLE};

typedef struct object {
        int   		type;
	unsigned char 	cnt;
	int		q;
        union { 
		float   floatpoint; 
                int     number; 
                char    *string;
                void    *cell;
                void    *dict;
                void    *stk;
		void    *fptr;
		void    *turtle;
		void    *part;
		oImage  *imgptr;
		fMatrix *fmat2;
                Motion  *mot;
      	};
} tOBJ;

extern int  freeobj	(tOBJ *b);
extern tOBJ emptyObj	();
extern tOBJ emptyTObj	(unsigned char t);
extern tOBJ emptyTPObj	(unsigned char t, void *p);
extern int  compareObj	(tOBJ a, tOBJ b);
extern tOBJ cloneObj	(tOBJ z);
extern tOBJ copyObj	(tOBJ z);


/**********************************************************/
/*  print                                                 */
/**********************************************************/

extern void printtype	(FILE *fp, tOBJ r);
extern tOBJ fprint	(FILE *fp, tOBJ r);
extern tOBJ print	(tOBJ r);
extern tOBJ println	(char *s,  tOBJ r);
extern int  sprint      (char *sp, tOBJ r);
extern void pp		(tOBJ x, int n);

/**********************************************************/
/*  conversions float   and integers                      */
/**********************************************************/

extern tOBJ  makefloat	    (float f);
extern tOBJ  makefloati     (int i);
extern float tofloat	    (tOBJ v);
extern tOBJ  makeint	    (int i);
extern int   toint	    (tOBJ v);

extern char *objtype	    (tOBJ t);

#endif
