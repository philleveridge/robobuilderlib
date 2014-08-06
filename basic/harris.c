
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
//  IMAGE HARRIS (IMAGE RAW "test.jpg")
extern void *readmatrix(char **str, char tc );

char *k = "1 0 -1;1 0 -1;1 0 -1]";

fMatrix *harris_detect(oImage * im)
{
	fMatrix *r=NULL;
	int nop=1;

	fMatrix *dx = readmatrix(&k, ']');
	//fMatrix *dx = gaussian_x(3,1); fmatprint2(dx);

	fMatrix *dy = ftranspose2(dx);
	oImage *ix  = imgconvmat(im, dx) ;
	oImage *iy  = imgconvmat(im, dy) ;
	oImage *ix2 = opImage(ix, ix, '*');
	oImage *iy2 = opImage(iy, iy, '*');
	oImage *ixy = opImage(ix, iy, '*');
/*
	corner measure = (Ix2.*Iy2 - Ixy.^2)./(Ix2 + Iy2 + epsilon); 

	    k = 0.04;
	    R11 = (Ix2.*Iy2 - Ixy.^2) - k*(Ix2 + Iy2).^2;
	    R11=(1000/max(max(R11)))*R11;
*/
	fMatrix *gk = gaussian2D(5, 1.0);
	oImage *sx2 = imgconvmat(ix2, gk);
	oImage *sy2 = imgconvmat(iy2, gk);
	oImage *sxy = imgconvmat(ixy, gk);

	if (dbg) ;  image2Pgm(sx2, "sx2.pgm");

	oImage *t1  = opImage(sx2, sy2, '*');
	oImage *t2  = opImage(sxy, sxy, '*');
	oImage *t3  = opImage(t1,  t2,  '-');

	oImage *t4  = opImage(sx2, sy2, '+');	
	oImage *cim = opImage(t3,  t4,  '/');	

	if (dbg) ; image2Pgm(cim, "cim.pgm");

	int loop=0;
	int count=0;
	int min_n=1, max_n=5;

	float threshold=20.0;

	while (((count<min_n) || (count>max_n)) && loop < 30)
	{
		loop++;
		if (count<min_n)
			threshold *= 0.5;
		if (count>max_n)
			threshold *= 1.5;
		
	}

	delimage (t1);
	delimage (t2);
	delimage (t3);
	delimage (t4);
	delimage (cim);
	
	r=newmatrix(2,nop);

	delmatrix(gk);
	delmatrix(dx);
	delmatrix(dy);
	delimage (ix);
	delimage (iy);
	delimage (ix2);
	delimage (iy2);
	delimage (ixy);
	delimage (sx2);
	delimage (sy2);
	delimage (sxy);

	return r;
}



