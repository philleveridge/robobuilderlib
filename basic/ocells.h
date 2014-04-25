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
extern tOBJ  	cnvtInttoList  	(int an, int *array);
extern tOBJ  	cnvtBytetoList 	(int an, BYTE *array);
extern tOBJ  	cnvtFloattoList	(int an, float *array);
extern int   	cnvtListtoByte 	(tOBJ lst, int an, BYTE *array);

#endif




