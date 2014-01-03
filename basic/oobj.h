#ifndef OOBJ_H
#define OOBJ_H

#include "fmatrix.h"

enum  TYPE  {SYM, INTGR, BOOLN, FUNC, FLOAT, STR, CELL, EMPTY, FMAT2, DICT, LAMBDA};

typedef struct object {
        int   type;
	unsigned char cnt;
        union { 
		int	q;
		float   floatpoint; 
                int     number; 
                char    *string;
                void    *cell;
                void    *dict;
		struct object	(*func)(struct object);
		fMatrix fmat2;
      	};
} tOBJ;

extern int  freeobj	(tOBJ *b);
extern tOBJ emptyObj	();
extern int  compareObj	(tOBJ a, tOBJ b);
extern tOBJ cloneObj	(tOBJ z);

/**********************************************************/
/*  print                                                 */
/**********************************************************/

extern void printtype	(FILE *fp, tOBJ r);
extern tOBJ fprint	(FILE *fp, tOBJ r);
extern tOBJ print	(tOBJ r);
extern tOBJ println	(char *s, tOBJ r);

/**********************************************************/
/*  conversuins float   and integers                      */
/**********************************************************/

extern tOBJ  makefloat	(float f);
extern tOBJ  makefloati	(int i);
extern float tofloat	(tOBJ v);
extern tOBJ  makeint	(int i);
extern int   toint	(tOBJ v);
extern tOBJ  cnvtInttoList(int an, int *array);
extern tOBJ  cnvtBytetoList(int an, BYTE *array);
extern tOBJ  cnvtFloattoList(int an, float *array);

#endif
