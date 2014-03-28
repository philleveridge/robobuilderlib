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
extern tOBJ 	makeCell3	(tOBJ a, tCELLp b);
extern tCELLp 	cloneCell	(tCELLp p);
extern tOBJ 	append		(tOBJ a, tOBJ b);

#endif




