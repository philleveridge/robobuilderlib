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



/**********************************************************/

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
	int i;
	int w=fgetw(ma);
	int h=fgeth(ma);
	fmatrixcreate(mb,w, h);
	float *p=fgetd(mb);

	if (dbg) printf ("sz=%d\n",h*w);

  	for (i=0; i<h*w; i++)
		p[i]=fgetd(ma)[i];
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


int ftranspose(char ln1, char lnx)   // @A^T"
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

int fconvolve(char ln1, char ln2)   // "@A (.) @B"
{	

	int w=fgetw(ln1);
	int h=fgeth(ln1);
	int mx,my;

	int szB=fsize(ln2,2);

	if (szB==9) // 3x3 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				float p=0.0f;
/*
				if (mx<w-1) 
					p += arrayA[my*w+mx+1]*arrayB[5];
				p += arrayA[my*w+mx+0]*arrayB[4];
				if (mx>0)   
					p += arrayA[my*w+mx-1]*arrayB[3];

				if (my>0) {
					if (mx<w-1) 
						p += arrayA[my*w+mx+1-w]*arrayB[2];
					p += arrayA[my*w+mx+0-w]*arrayB[1];
					if (mx>0)   
						p += arrayA[my*w+mx-1-w]*arrayB[0];
				}

				if (my<h-1) {
					if (mx<w-1) 
						p += arrayA[my*w+mx+1+w]*arrayB[9];
					p += arrayA[my*w+mx+0+w]*arrayB[8];
					if (mx>0)   
						p += arrayA[my*w+mx-1+w]*arrayB[7];
				}
*/

				if (mx==0 || mx==w-1 || my==0 || my==h-1)
					scene[my*w+mx]=0;
				else
					scene[my*w+mx]=abs((int)p);
			}
		}
	}
	
	if (szB==2 && fmstore[ln2-'A'].w==2 ) // 2x1 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				if (mx<w-1) 
					//scene[my*w+mx]= abs(arrayA[my*w+mx]*arrayB[0] + arrayA[my*w+mx+1]*arrayB[1]);
					scene[my*w+mx]= abs(fget(ln1,mx,my)*fget(ln2,0,0) + fget(ln1,mx+1,my)*fget(ln2,1,0));

				else
					scene[my*w+mx]= 0;
			}
		}
	}

	if (szB==2 && fmstore[ln2-'A'].w==1 ) //  1x2 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				if (my<h-1) 
					//scene[my*w+mx]= abs(arrayA[my*w+mx]*arrayB[0] + arrayA[(my+1)*w+mx]*arrayB[1]);
					scene[my*w+mx]= abs(fget(ln1,mx,my)*fget(ln2,0,0) + fget(ln1,mx,my+1)*fget(ln2,0,1));
				else
					scene[my*w+mx]= 0;
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
				scene[my*w+mx]=abs(p);  // tbd
			}
		}
	}
	nis=h*w;
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
