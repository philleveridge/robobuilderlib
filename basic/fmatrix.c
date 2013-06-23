#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "win.h"
int tp[10000]; //windows temp fix
#endif

#ifdef LINUX
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "linux.h"

//#define NULL (void *)0
#endif


#include "express.h"
#include "functions.h"
#include "lists.h"

int abs(int);

extern int  dbg;

#define MATSIZE   10000

fMatrix fmstore[26];
float  fdata   [MATSIZE];

int fm=0;
int fs=0;
static int initflag=1;

/**********************************************************/



/**********************************************************

New Matrix 

**********************************************************/

fMatrix newmatrix(int c, int r)
{
	fMatrix n;

	if (dbg) printf ("New Matrix %d,%d\n",c,r);
	n.h=r;
	n.w=c;
	n.fstore = 0;

	//allocate space

	n.fstore = (float *)malloc(r*c*sizeof(float));

	if (n.fstore==0)
	{
		printf ("out of space %d (%d)\n",fs,MATSIZE);
	}
	else
	{
		for (int i=0; i<r*c; i++) n.fstore[i]=0.0;
	}

	return n;
}

int delmatrix(fMatrix *m)
{
	if (dbg) printf ("Del Matrix [%d,%d]\n", m->w, m->h);

	if (m->h<=0 || m->w<=0 || m->fstore==0)
		return 0;

	m->h=0;
	m->w=0;	
	free(m->fstore);
	return 0;
}

fMatrix fmatcp(fMatrix *A) // clone
{
	fMatrix n;
	int i;
	int w=A->w;
	int h=A->h;

	n = newmatrix(w, h);

	if (dbg) printf ("sz=%d\n",h*w);

  	for (i=0; i<h*w; i++)
		n.fstore[i]=A->fstore[i];

	return n;
}

fMatrix fmatrshp(fMatrix *A, int c, int r) //clone &  resize
{
	fMatrix n;
	int i, j;
	int w=A->w;
	int h=A->h;

	n = newmatrix(c, r);

  	for (i=0; i<r; i++)
		for (j=0; j<c; j++)
		{
			if (i<h && j<w)
				n.fstore[j+i*c]=A->fstore[j+i*w];
			else
				n.fstore[j+i*c]=0.0;
		}

	return n;
}

int fmatprint2(fMatrix *A)
{
	int i,j;
	printf ("Matrix %dx%d\n", A->w, A->h);

	for (j=0; j<A->h;j++)
	{
		for (i=0; i<A->w; i++)
		{
			printf ("%3f ", A->fstore[i+j*A->w]);
		}
		printf ("\n");
	}
	return 0;
}

float fget2(fMatrix *M, int c, int r)
{
	if (c>=0 && r>=0 && c<M->w && r<M->h && M->fstore!=0)
	{
		//printf ("ok- %d, %d %f\n", M->w, M->h, M->fstore[c+r*M->w]);
		return  M->fstore[c+r*(M->w)];
	}
	else
	{
		printf ("err- %d, %d\n", M->w, M->h);
		return 0.0;
	}
}

float fset2(fMatrix *M, int c, int r, float v)
{
	if (c>=0 && r>=0 && c<M->w && r<M->h && M->fstore!=0)
		M->fstore[c+r*M->w] =v;
}

fMatrix fadd2(fMatrix *A, fMatrix *B, char op)  
{
	int rx,ry, ha,wa,wb,hb;

	fMatrix R;
	R.h=0;
	R.w=0;

	wa=A->w;
	ha=A->h;
	wb=B->w;
	hb=B->h;

	if (wa!=wb || ha!=hb)
		return R;

	R = newmatrix(wa,ha);

	for (ry=0; ry<ha; ry++)
	{
		for (rx=0; rx<wa; rx++)
		{
			if (op=='+') R.fstore[rx+ry*wa] = fget2(A,rx,ry) + fget2(B,rx,ry);  
			if (op=='-') R.fstore[rx+ry*wa] = fget2(A,rx,ry) - fget2(B,rx,ry);  
			if (op=='/') R.fstore[rx+ry*wa] = fget2(A,rx,ry) / fget2(B,rx,ry);  
			if (op=='*') R.fstore[rx+ry*wa] = fget2(A,rx,ry) * fget2(B,rx,ry);  
		}
	}
	return R;
}

fMatrix ftranspose2(fMatrix *A)  
{
	int w=A->w;
	int h=A->h;
	int mx,my;
	fMatrix R;

	if (dbg) printf ("transpose %c,%c\n",w,h);

	R = newmatrix(h, w);

	for (my=0; my<h; my++)
	{
		for (mx=0; mx<w; mx++)
		{
			R.fstore[mx*h+my] = A->fstore[mx+my*w];
		}
	}
	return R;
}


fMatrix fmultiply2(fMatrix *A,fMatrix *B)   
{
	int m,rx,ry, ha,wa,hb,wb;
	fMatrix R; 

	wa=A->w;
	ha=A->h;
	wb=B->w;
	hb=B->h;

	if (wa != hb) 
	{
		printf ("Bad Matrix size (%d,%d) * (%d,%d) \n",wa,ha,wb,hb);
		R.w=0; R.h=0; R.fstore=0;
		return R; //bad array size
	}

	R = newmatrix(wb, ha);

	for (ry=0; ry<ha; ry++)
	{
		for (rx=0; rx<wb; rx++)
		{
			R.fstore[rx+ry*wb] = 0.0f;
			for (m=0; m<hb; m++)
			{
				//tp[rx+ry*wb] += fget(ln1,m,ry) * fget(ln2,rx,m);  
				R.fstore[rx+ry*wb] += A->fstore[m+ry*wa] * B->fstore[rx+m*wb];
			}
		}
	}
	return R;
}

fMatrix freplicate2(fMatrix *A, int m, int n)
{
	fMatrix R;
 	int i,j,x,y;
	int w=A->w;
	int h=A->h;
	int wx=w*m;
	int hx=h*n;

	if (dbg) printf ("rep [%d,%d]\n", m,n);

	R = newmatrix(wx, hx);

	float *p =A->fstore;
	float *pt=R.fstore;

	for (j=0;j<h;j++)
		for (i=0;i<w;i++)
			for (x=0;x<m;x++)
				for (y=0;y<n;y++) 
				{
					pt[i+j*wx+y*wx+x*w] = p[i+j*w];
				}
	return R;
}


fMatrix fmatsum2(fMatrix *A, int mode)   
{	
	fMatrix R;
	int w=A->w;
	int h=A->h;
	int mx,my;
	float p=0.0;
	float t=0.0;

	if (mode==1 || mode==3)
	{
		R = newmatrix(1, h);  

		for (my=0;my<h; my++)
		{
			p=0.0;
			for (mx=0;mx<w; mx++)
			{
				t = fget2(A, mx,my);
				if (mode==2) t=t*t;
				p += t;
			}				
			fset2(&R,0,my,p);
		}
	}
	else if (mode==2 || mode==4)
	{
		R = newmatrix(w, 1);

		for (mx=0;mx<w; mx++)
		{
			p=0.0;
			for (my=0;my<h; my++)
			{
				t = fget2(A, mx,my);
				if (mode==4) t=t*t;
				p += t;
			}				
			fset2(&R,mx,0,p);
		}
	}
	return R;
}


fMatrix fconvolve2(fMatrix *A, fMatrix *B)  
{	
	fMatrix R;
	int w=A->w;
	int h=A->h;
	int mx,my;

	int szB=(B->w)*(B->h);	

	R = newmatrix(w, h);  

	if (szB==9) // 3x3 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				float p=0.0f;

				p += fget2(A,mx-1,my-1)*fget2(B,0,0);
				p += fget2(A,mx,my-1)  *fget2(B,1,0);
				p += fget2(A,mx+1,my-1)*fget2(B,2,0);

				p += fget2(A,mx-1,my)  *fget2(B,0,1);
				p += fget2(A,mx,my)    *fget2(B,1,1);
				p += fget2(A,mx+1,my)  *fget2(B,2,1);

				p += fget2(A,mx-1,my+1)*fget2(B,0,2);
				p += fget2(A,mx,my+1)  *fget2(B,1,2);
				p += fget2(A,mx+1,my+1)*fget2(B,2,2);

				fset2(&R,mx,my,p);
			}
		}
	}
	
	if (szB==2 && B->w==2 ) // 2x1 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				fset2(&R,mx,my, abs(fget2(A,mx,my)*fget2(B,0,0) 
					+ fget2(A,mx+1,my)*fget2(B,1,0)));
			}
		}
	}

	if (szB==2 && B->w==1 ) //  1x2 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				fset2(&R,mx,my,abs(fget2(A,mx,my)*fget2(B,0,0) + fget2(A,mx,my+1)*fget2(B,0,1)));
			}
		}
	}

	if (szB==4) // 2x2 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				int p=0; //tbd
			}
		}
	}

	return R;	
}

fMatrix   fimport2(char m2, int c, int r)
{
	fMatrix R;
	int i, j, n=0;
	R = newmatrix(c, r); 

	for (i=0; i<r; i++)
	{
		for (j=0; j<c; j++)
			fset2(&R, j, i, (float)listread(m2,n++));
	}
	return R;
}	

fMatrix fmatzerodiag2(fMatrix *A)   
{	
	fMatrix R;
	int i, h=A->h;
	R = newmatrix(A->w, h);  

	for (i=0; i<h; i++)
	{
		fset2(&R,i,i,0.0f);
	}
	return R;
}
