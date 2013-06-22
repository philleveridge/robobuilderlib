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

fMatrix fadd2(fMatrix *A, fMatrix *B, char op)   // "@X = @A + @B"
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

	if (op=='+')
	{
		for (ry=0; ry<ha; ry++)
		{
			for (rx=0; rx<wa; rx++)
			{
				R.fstore[rx+ry*wa] = fget2(A,rx,ry) + fget2(B,rx,ry);  
			}
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

	for (ry=0; ry<ha; ry++)
	{
		for (rx=0; rx<wb; rx++)
		{
			//fset(lnx,rx, ry, tp[rx*ha+ry]);	
		}
	}
	return R;
}




/**********************************************************

OLD style matrix functions

**********************************************************/

void initmatrix()
{
	int i;
	//printf ("Matrix init\n");
	for (i=0; i<26; i++)
	{
		fmstore[i].h=0;
		fmstore[i].w=0;
		fmstore[i].fstore=(float *)0;
	}
	initflag=0;
}

int fgetw(char m)
{
	return fmstore[m-'A'].w;
}

int fgeth(char m)
{
	return fmstore[m-'A'].h;
}

float *fgetd(char m)
{
	return &fmstore[m-'A'].fstore[0];
}

int fsize(char m, int p) //p=0 return w else return h
{
	if (m>='A' && m<='Z')
	{
		switch (p) {
		case 0:  return fgetw(m);
		case 1:  return fgeth(m);
		case 2:  return fgeth(m)*fgetw(m);
		default: return 0;
		}
	}
	return 0;
}


int fmatrixcreate(char ma, int w, int h)
{
	int m;
	if (initflag)
		initmatrix();

	m=ma-'A';
	if (m<0 || m>25 || w<=0 || h<=0)
	{
		printf ("Param error\n");
		return 1;
	}

	if (fgetw(m)>0 && h*w <= fgetw(m)*fgeth(m) )
	{
		if (dbg) printf ("Re-use Space\n");
		fmstore[m].w=w;
		fmstore[m].h=h;
		return 0;
	}

	//allocate space

	if (fs+h*w<MATSIZE)
	{
		fmstore[m].fstore = &fdata[fs];
		fm=fm+1;
		fs=fs+h*w;
		fmstore[m].w=w;
		fmstore[m].h=h;
		return 0;
	}
	else
	{
		printf ("out of space %d (%d)\n",fs,MATSIZE);
		return 1;
	}
}

int fmatcopy(char ma, char mb)
{
	//ma -> mb
	float *p;
	int i;
	int w=fgetw(ma);
	int h=fgeth(ma);

	int wb=fgetw(mb);
	int hb=fgeth(mb);

	if (wb<w || h<hb) fmatrixcreate(mb,w, h);
	p=fgetd(mb);

	if (dbg) printf ("sz=%d\n",h*w);

  	for (i=0; i<h*w; i++)
		p[i]=fgetd(ma)[i];
	return 0;
}


/**********************************************************

 ma (x, y) =< mx (m*x, n*y)
**********************************************************/

int freplicate(char mx, char ma, int m, int n)
{
	int i,j,x,y;
	int w=fgetw(ma);
	int h=fgeth(ma);
	int wx=w*m;
	int hx=h*n;

	if (dbg) printf ("rep %c = %d [%d,%d]\n", mx,ma,m,n);

	fmatrixcreate(mx, wx, hx);

	float *p =fgetd(ma);
	float *pt=fgetd(mx);

	for (j=0;j<h;j++)
		for (i=0;i<w;i++)
			for (x=0;x<m;x++)
				for (y=0;y<n;y++) 
				{
					pt[i+j*wx+y*wx+x*w] = p[i+j*w];
				}
	return 0;
}

float fget(char m,int w,int h)
{
	if (m>='A' && m<='Z' && h>=0 && w>=0 && h<fgeth(m) && w<fgetw(m))
	{
		int n=h+fgeth(m)*w;
		float f=fmstore[m-'A'].fstore[n];

		if (dbg) printf ("GET %c (%d,%d)=%f\n",m,w,h,f);

		return f;
	}
	return 0.0;
}

int fset(char m,int w,int h, float v)
{
	if (dbg) printf ("SET %c (%d,%d)=%f\n",m,w,h,v);

	if (m>='A' && m<='Z' && h>=0 && w>=0 && h<fgeth(m) && w<fgetw(m))
	{
		int n=h+fgeth(m)*w;
		fmstore[m-'A'].fstore[n] = v;
		return 0;
	}
	else
		printf ("Param error %d,%d\n",w,h);
	return 1;
}


int fimport(char m, char m2)
{
	int i;

	if (m>='A' && m<='Z')
	{
		int n=fsize(m,2);
		float *fp=fgetd(m);

		for (i=0; i<n; i++)
		{
			fp[i] = (float)listread(m2,i);
		}
		return 0;
	}
	else
	{
		printf ("Param error %c<-%d\n",m,m2);
		return 1;
	}
}

int fimportf(char m, char m2, float rd)
{
	int i,j;
	//copy m2 -> m and pad with cd or rd
		
	if (m>='A' && m<='Z' && m2>='A' && m2<='Z')
	{
		int h1=fgeth(m);
		int w1=fgetw(m);
		int h2=fgeth(m2);
		int w2=fgetw(m2);

		for (i=0; i<h1; i++)
		{
			for (j=0; j<w1; j++)
			{
				float f=0.0;
				if (i>=h2 || j>=w2) 
					f=rd;
				else
					f=fget(m2,j,i);
				fset(m,j,i,f);
			}
		}
		return 0;
	}
	else
	{
		printf ("Param error %c<-%d\n",m,m2);
		return 1;
	}
}

/**********************************************************/
//
//
/**********************************************************/

int fmatzerodiag(char ln1)   
{
	int i, h=fgeth(ln1);

	for (i=0; i<=h; i++)
	{
		fset(ln1,i,i,0.0f);
	}
	return 0;
}

/**********************************************************/
//
//
/**********************************************************/

int fmatzero(char ln1, int a, int b, int c, int d)   
{
	int i, j;

	int w=fgetw(ln1);
	int h=fgeth(ln1);

	if (a<0 || b<0 || c>=w || d>=h) 
	{
		printf ("Bad parameter zero @%c[%d,%d - %d,%d]\n",ln1,a,b,c,d);
		return 1; //bad array size
	}

	for (i=a; i<=c; i++)
	{
		for (j=b; j<=d; j++)
		{
			fset(ln1,i,j,0.0f);
		}
	}
	return 0;			
}

/**********************************************************/
//
//
/**********************************************************/

int fmatnorm(char ln1)   
{
	int i, j;

	int w=fgetw(ln1);
	int h=fgeth(ln1);

	// find max
	float max=0.0;

	for (i=0; i<w; i++)
	{
		for (j=0; j<h; j++)
		{
			float v=fget(ln1,i,j);
			if (i==0 && j==0) max=v;
			else
			if (v>max) max=v;
		}
	}

	for (i=0; i<w; i++)
	{
		for (j=0; j<h; j++)
		{
			fset(ln1,i,j,fget(ln1,i,j)/max);
		}
	}
	return 0;			
}

/**********************************************************/
//
//
/**********************************************************/

int fmatprint(char m)
{
	int i,j;
	int w=fgetw(m);
	int h=fgeth(m);
	printf ("matrix '%c' %dx%d\n", m, w, h);

	for (j=0; j<h;j++)
	{
		for (i=0; i<w; i++)
		{
			printf ("%3f ", fget(m,i,j));
		}
		printf ("\n");
	}
	return 0;
}

/******************************

A    = 1 2 3
       4 5 6

T(A) = 1 4
       2 5
       3 6

*******************************/


int ftranspose(char ln1, char lnx)  
{
	int w=fgetw(ln1);
	int h=fgeth(ln1);
	int mx,my;

#ifndef WIN32
	float tp[w*h]; //temp
#endif
	if (dbg) printf ("transpose=%c->%c\n",ln1,lnx);

	for (my=0; my<h; my++)
	{
		for (mx=0; mx<w; mx++)
		{
			tp[mx*h+my] = fget(ln1,mx,my);	
		}
	}
	
	if (fmatrixcreate(lnx, h, w)<0) 
	{
		printf("Err: listcreate failed\n");
		return 1;
	}

	for (my=0; my<h; my++)
	{
		for (mx=0; mx<w; mx++)
		{
			fset(lnx,my,mx, tp[mx*h+my]);	
		}
	}
	return 0;
}


/**********************************************************/
//
//
/**********************************************************/

int fmultiply(char ln1, char ln2, char lnx)   // "@X = @A * @B"
{
	int m,rx,ry, ha,wa,hb,wb;
 
	wa=fgetw(ln1);
	ha=fgeth(ln1);
	wb=fgetw(ln2);
	hb=fgeth(ln2);

	// ha,wa * hb,wb
	// wa == hb
      	// Mresult =[ha*wb]

#ifndef WIN32
	float tp[wb*ha]; //temp
#endif

	if (wa != hb) 
	{
		printf ("Bad Matrix size (%d,%d) * (%d,%d) \n",wa,ha,wb,hb);
		return 2; //bad array size
	}

	for (ry=0; ry<ha; ry++)
	{
		for (rx=0; rx<wb; rx++)
		{
			tp[rx+ry*wb] = 0.0f;
			for (m=0; m<hb; m++)
			{
				tp[rx+ry*wb] += fget(ln1,m,ry) * fget(ln2,rx,m);  
			}
		}
	}

	if (fmatrixcreate(lnx, wb, ha)<0)
	{
		printf ("Can't create (%d,%d) \n",wb,ha);
		return 1;
	}

	for (ry=0; ry<ha; ry++)
	{
		for (rx=0; rx<wb; rx++)
		{
			fset(lnx,rx, ry, tp[rx*ha+ry]);	
		}
	}
	return 0;
}

/**********************************************************/
//
//
/**********************************************************/

int fadd(char ln1, char ln2, char lnx, char op)   // "@X = @A + @B"
{
	int rx,ry, ha,wa,wb;

	wa=fgetw(ln1);
	ha=fgeth(ln1);
	wb=fgetw(ln2);

#ifndef WIN32
	float tp[wb*ha]; //temp
#endif

	if (op=='+')
	{
		for (ry=0; ry<ha; ry++)
		{
			for (rx=0; rx<wa; rx++)
			{
				tp[rx+ry*wa] = fget(ln1,rx,ry) + fget(ln2,rx,ry);  
			}
		}
	}

	if (op=='-')
	{
		for (ry=0; ry<ha; ry++)
		{
			for (rx=0; rx<wa; rx++)
			{
				tp[rx+ry*wa] = fget(ln1,rx,ry) - fget(ln2,rx,ry);  
			}
		}
	}

	if (op=='*')
	{
		for (ry=0; ry<ha; ry++)
		{
			for (rx=0; rx<wa; rx++)
			{
				tp[rx+ry*wa] = fget(ln1,rx,ry) * fget(ln2,rx,ry);  
			}
		}
	}

	if (op=='/')
	{
		for (ry=0; ry<ha; ry++)
		{
			for (rx=0; rx<wa; rx++)
			{
				tp[rx+ry*wa] = fget(ln1,rx,ry) / fget(ln2,rx,ry);  
			}
		}
	}

	if (fmatrixcreate(lnx, wa, ha)<0)
	{
		printf ("Can't create (%d,%d) \n",wb,ha);
		return 1;
	}

	for (ry=0; ry<ha; ry++)
	{
		for (rx=0; rx<wa; rx++)
		{
			fset(lnx,rx, ry, tp[rx+ry*wa]);	
		}
	}
	return 0;
}



/**********************************************************/
//
//
/**********************************************************/

int fconvolve(char ln1, char ln2, char lnx)   // "@A (.) @B"
{	

	int w=fgetw(ln1);
	int h=fgeth(ln1);
	int mx,my;

	int szB=fsize(ln2,2);

#ifndef WIN32
	float tp[w*h]; //temp
#endif

	if (szB==9) // 3x3 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				float p=0.0f;

				p += fget(ln1,mx-1,my-1)*fget(ln2,0,0);
				p += fget(ln1,mx,my-1)  *fget(ln2,1,0);
				p += fget(ln1,mx+1,my-1)*fget(ln2,2,0);

				p += fget(ln1,mx-1,my)  *fget(ln2,0,1);
				p += fget(ln1,mx,my)    *fget(ln2,1,1);
				p += fget(ln1,mx+1,my)  *fget(ln2,2,1);

				p += fget(ln1,mx-1,my+1)*fget(ln2,0,2);
				p += fget(ln1,mx,my+1)  *fget(ln2,1,2);
				p += fget(ln1,mx+1,my+1)*fget(ln2,2,2);

				tp[my*w+mx]=abs((int)p);
			}
		}
	}
	
	if (szB==2 && fgetw(ln2)==2 ) // 2x1 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				tp[my*w+mx]= abs(fget(ln1,mx,my)*fget(ln2,0,0) + fget(ln1,mx+1,my)*fget(ln2,1,0));
			}
		}
	}

	if (szB==2 && fgetw(ln2)==1 ) //  1x2 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				tp[my*w+mx]= abs(fget(ln1,mx,my)*fget(ln2,0,0) + fget(ln1,mx,my+1)*fget(ln2,0,1));
			}
		}
	}

	if (szB==4) // 2x2 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				int p=0;
				tp[my*w+mx]=abs(p);  // tbd
			}
		}
	}

	for (my=0;my<h; my++)
	{
		for (mx=0;mx<w; mx++)
		{
			fset(lnx,mx,my,tp[my*w+mx]); 
		}
	}

	return 0;	
}

/**********************************************************/
//
//
/**********************************************************/

int fmatsum(char ln2, char ln1, int mode)   
{
	int w=fgetw(ln1);
	int h=fgeth(ln1);
	int mx,my;
	float p=0.0;
	float t=0.0;

	if (mode==1 || mode==3)
	{
		for (my=0;my<h; my++)
		{
			p=0.0;
			for (mx=0;mx<w; mx++)
			{
				t = fget(ln1, mx,my);
				if (mode==2) t=t*t;
				p += t;
			}				
			fset(ln2,0,my,p);
		}
	}
	else if (mode==2 || mode==4)
	{
		for (mx=0;mx<w; mx++)
		{
			p=0.0;
			for (my=0;my<h; my++)
			{
				t = fget(ln1, mx,my);
				if (mode==4) t=t*t;
				p += t;
			}				
			fset(ln2,mx,0,p);
		}
	}
	return 0;
}


/**********************************************************/
//
//
/**********************************************************/

int fhistogram(char ln1, int mode)   // "@A 
{
	int w=fgetw(ln1);
	int h=fgeth(ln1);
	int mx,my;

	if (mode==1)
	{
		for (my=0;my<h; my++)
		{
			float p=0.0;
			for (mx=0;mx<w; mx++)
			{
				p += fget(ln1, mx,my); //[my*w+mx];	
			}				
			scene[my]=(int)p;
		}
		nis=h;
	}
	else
	{
		for (mx=0;mx<w; mx++)
		{
			float p=0.0;
			for (my=0;my<h; my++)
			{
				p += fget(ln1, mx,my); //arrayA[my*w+mx];	
			}				
			scene[mx]=p;
		}
		nis=w;
	}
	return 0;
}
