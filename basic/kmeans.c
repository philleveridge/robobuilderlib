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

//IMAGE KMEANS {200 0 200;200 0 200; 200 0 200} 2

fMatrix *kmeans_mat(fMatrix *fm, int k)
{
	int i,j;
	if (fm==NULL) return NULL;

	if (fm->h < k) return NULL;  // must be more points than k !

	fMatrix *rm = newmatrix(3,k+fm->h);	// results

	// initialise
	int no_iterations=0;

	for (i=0; i<k; i++)
	{
		float x=fget2(fm,0,i);
		float y=fget2(fm,1,i);
		fset2(rm,0,i,x); 
		fset2(rm,1,i,y); 
		fset2(rm,2,i,0); 
	}

	//loop

	while (no_iterations<10)
	{
		float cx[k], cy[k];
		no_iterations++;
		for (i=0; i<fm->h; i++)
		{
			float mx,my,sd=10000.0;
			int bk=0;
			float x=fget2(fm,0,i);
			float y=fget2(fm,1,i);
			for (j=0; j<k; j++)
			{
				float d = dist(x,y, fget2(rm,0,j), fget2(rm,1,j));
				if (d<sd) { sd=d; bk=j; }
				//printf ("%d %d %f\n", i,j, d);
			}
			printf ("assigned to k=%d\n", bk);
			fset2(rm,0,k+i,fget2(fm,0,i)); 
			fset2(rm,1,k+i,fget2(fm,1,i)); 
			fset2(rm,2,k+i,bk);
			fmupdate(rm,2,bk,1); 
		}

		for (i=0; i<k; i++)
		{
			cx[i]=0.0;
			cy[i]=0.0;
		}

		for (i=0; i<fm->h; i++)
		{
			// find new centre
			float tx = fget2(rm,0,k+i);
			float ty = fget2(rm,1,k+i);
			int tk = (int)fget2(rm,2,k+i);

			printf ("%d - %f %f\n", tk,tx,ty);

			cx[tk] += tx;
			cy[tk] += ty;
		}

		float dif = 0.0;
		for (i=0; i<k; i++)
		{
			// find new centre
			cx[i] /= fget2(rm,2,i); 
			cy[i] /= fget2(rm,2,i); 

			dif += pow((cx[i]-fget2(rm,0,i)),2) + pow((cy[i]-fget2(rm,1,i)),2);
		}

		printf ("dif= %f\n", dif);
		if (dif <0.1) break;

		for (i=0; i<k; i++)
		{
			// find new centre
			fset2(rm,0,i,cx[i]); 
			fset2(rm,1,i,cy[i]); 
			fset2(rm,2,i,0); 
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
	if (img==NULL) return NULL;

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

