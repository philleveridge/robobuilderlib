#ifndef OOBJ_H
#define OOBJ_H

#include "macro.h"
#include "fmatrix.h"
#include "rbmread.h"

extern int dbg;


enum  TYPE  {SYM, INTGR, BOOLN, FUNC, FLOAT, CELL, EMPTY, FMAT2, DICT, LAMBDA, RBM, STACK};

typedef struct object {
        unsigned char  	type;
	unsigned char 	cnt;
	unsigned char	q;
        union { 
		float   floatpoint; 
                int     number; 
                char    *string;
                void    *cell;
                void    *dict;
                void    *stk;
		void    *fptr;
		fMatrix *fmat2;
                Motion  *mot;
      	};
} tOBJ;

extern int  freeobj	(tOBJ *b);
extern tOBJ emptyObj	();
extern int  compareObj	(tOBJ a, tOBJ b);
extern tOBJ cloneObj	(tOBJ z);
extern tOBJ copyObj	(tOBJ z);

/**********************************************************/
/*  print                                                 */
/**********************************************************/

extern tOBJ print	(tOBJ r);
extern tOBJ println	(char *s,  tOBJ r);
extern void pp		(tOBJ x, int n);

/**********************************************************/
/*  conversions float   and integers                      */
/**********************************************************/

extern tOBJ  makefloat	    (float f);
extern tOBJ  makefloati     (int i);
extern float tofloat	    (tOBJ v);
extern tOBJ  makeint	    (int i);
extern int   toint	    (tOBJ v);
extern tOBJ  cnvtInttoList  (int an, int *array);
extern tOBJ  cnvtBytetoList (int an, BYTE *array);
extern tOBJ  cnvtFloattoList(int an, float *array);
extern int   cnvtListtoByte (tOBJ lst, int an, BYTE *array);
extern tOBJ  makenumfstr    (char *s);

extern char *objtype	    (tOBJ t);

#endif
