#ifndef OCELL_H
#define OCELL_H

#include "oobj.h"

/**********************************************************/
/*  CELLS                                                 */
/**********************************************************/

typedef struct cell { 
        tOBJ            head;           
        struct cell*    tail;           
} tCELL, *tCELLp; 

extern tCELLp 	newCell		();
extern void 	delCell		(tCELLp p);
extern tOBJ 	makeCell	();
extern tOBJ 	makeCell2	(tOBJ a, tCELLp b);
extern tCELLp 	cloneCell	(tCELLp p);
extern tOBJ 	append		(tOBJ a, tOBJ b);

extern tOBJ 	cnvtDDtoList	(double a, double b);
extern tOBJ 	cnvtIItoList	(int a, int b);
extern tOBJ 	cnvtOOtoList	(tOBJ a, tOBJ b);
extern tOBJ 	cnvtOOOtoList	(tOBJ a, tOBJ b, tOBJ c);
extern tOBJ 	cnvtDDDtoList	(double a, double b, double c);

extern tOBJ  	cnvtInttoList  	(int an, int *array);
extern tOBJ  	cnvtBytetoList 	(int an, BYTE *array);
extern tOBJ  	cnvtFloattoList	(int an, float *array);

extern int   	cnvtListtoByte 	(tOBJ lst, int an, BYTE  *array);
extern int 	cnvtListtoInt	(tOBJ lst, int an, int   *array);
extern int 	cnvtListtoFloat	(tOBJ lst, int an, float *array);

#endif




