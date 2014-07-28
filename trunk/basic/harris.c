
/*
% Harris detector
% The code calculates
% the Harris Feature Points(FP) 
*/

#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "win.h"
#endif

#ifdef LINUX
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "linux.h"
#endif

#include "harris.h"

extern void *readmatrix(char **str, char tc );

char *k = "1 0 -1;1 0 -1;1 0 -1]";

fMatrix *harris_detect(oImage * im)
{
	fMatrix *r=NULL;
	int nop=1;

	fMatrix *dx = readmatrix(&k, ']');
	fMatrix *dy = ftranspose2(dx);

	oImage *ix = imgconvmat(im, dx) ;
	oImage *iy = imgconvmat(im, dy) ;

	oImage *ix2 = opImage(ix, ix, '*');
	oImage *iy2 = opImage(iy, iy, '*');
	oImage *ixy = opImage(ix, iy, '*');
/*
	corner measure = (Ix2.*Iy2 - Ixy.^2)./(Ix2 + Iy2 + epsilon); 
*/

	oImage *t1  = opImage(ix2, iy2, '*');
	oImage *t2  = opImage(ixy, ixy, '*');
	oImage *t3  = opImage(t1,  t2,  '-');

	oImage *t4  = opImage(ix2, iy2, '+');	
	oImage *cim = opImage(t3,  t4,  '/');	

	delimage (t1);
	delimage (t2);
	delimage (t3);
	delimage (t4);
	delimage (cim);
	
	r=newmatrix(2,nop);

	delmatrix(dx);
	delmatrix(dy);
	delimage (ix);
	delimage (iy);
	delimage (ix2);
	delimage (iy2);
	delimage (ixy);

	return r;
}



