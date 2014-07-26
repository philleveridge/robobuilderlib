//kmeans

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

#include "kmeans.h"

float dist(float a1,float b1,float a2,float b2)
{
	return sqrt((a1-a2)*(a1-a2)+(b1-b2)*(b1-b2));	
}

fMatrix *kmeans_mat(fMatrix *fm, int k)
{

	if (fm==NULL) return NULL;

	fMatrix *rm = newmatrix(2,k);	

	// code
	int no_iterations=0;

	

	return rm;
}

fMatrix *kmeans_img(oImage *img, int k, int th)
{
	fMatrix *rm, *nm;
	int i,p = 0;
	int cnt=0;

	// setup nm

	for (i=0; i<img->w*img->h; i++) 
		if (img->data[i]>th) cnt++;

	nm = newmatrix(3,cnt);

	for (i=0; i<img->w*img->h; i++) 
	{
		if (img->data[i]>th)
		{
			fset2(nm,0,p, i % img->w); 
			fset2(nm,1,p, i / img->w); 
			fset2(nm,2,p, img->data[i]); 
			p++;
		}
	}

	if (dbg) fmatprint2(nm);

	rm = kmeans_mat(nm, k);
	delmatrix(nm);
	return rm;
}

