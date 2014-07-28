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
	float r= sqrt((a1-a2)*(a1-a2)+(b1-b2)*(b1-b2));	
	return r;
}

fMatrix *kmeans_mat(fMatrix *fm, int k)
{
	int i,j;
	if (fm==NULL) return NULL;

	if (fm->h < k) return NULL;  // must be more points than k !

	fMatrix *rm = newmatrix(2,k);	// results

	// initialise
	int no_iterations=0;
	float max_delta=0.001;
	float delta=1.0;

	for (i=0; i<k; i++)
	{
		float x=fget2(fm,0,i);
		float y=fget2(fm,1,i);
		fset2(rm,0,i,x); 
		fset2(rm,1,i,y); 
	}

	//loop
	while (no_iterations<10 && delta >max_delta)
	{
		no_iterations++;
		for (i=k; i<fm->h; i++)
		{
			float mx,my;
			float x=fget2(fm,0,i);
			float y=fget2(fm,1,i);
			for (j=0; j<k; j++)
			{
				printf ("%d %d %f\n", i,j, dist(x,y, fget2(rm,0,j), fget2(rm,0,j)));
			}
		}
	}

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

