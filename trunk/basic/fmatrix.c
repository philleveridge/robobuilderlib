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

typedef struct fmat {
	int h; 
	int w;
	float *fstore;
} fMatrix;

fMatrix fmstore[26];
float  fdata   [10000];

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

	if (fmstore[m].w>0 && h*w <= fmstore[m].w*fmstore[m].h )
	{
		if (dbg) printf ("Re-use Space\n");
		fmstore[m].w=w;
		fmstore[m].h=h;
		return 0;
	}

	//allocate space

	if (fs+h*w<10000)
	{
		fmstore[m].fstore = &fdata[fs];
		fm=fm+1;
		fs=fs+h*w;
if (dbg) printf ("fs=%d\n",fs);
		fmstore[m].w=w;
		fmstore[m].h=h;
		return 0;
	}
	else
	{
		printf ("out of space\n");
		return 1;
	}
}

float fget(char m,int w,int h)
{
	if (m>='A' && m<='Z' && h>=0 && w>=0 && h<fmstore[m-'A'].h && w<fmstore[m-'A'].w)
	{
		int n=h+fmstore[m-'A'].h*w;
		float f=fmstore[m-'A'].fstore[n];

if (dbg) printf ("GET %c (%d,%d)=%f\n",m,w,h,f);

		return f;
	}
//	else
//		printf ("Param error %d,%d\n",w,h);
	return 0.0;
}

int fset(char m,int w,int h, float v)
{
if (dbg) printf ("SET %c (%d,%d)=%f\n",m,w,h,v);

	if (m>='A' && m<='Z' && h>=0 && w>=0 && h<fmstore[m-'A'].h && w<fmstore[m-'A'].w)
	{
		int n=h+fmstore[m-'A'].h*w;
		fmstore[m-'A'].fstore[n] = v;
		return 0;
	}
	else
		printf ("Param error %d,%d\n",w,h);
	return 1;
}

int fsize(char m, int p) //p=0 return w else return h
{
	if (m>='A' && m<='Z')
	{
		switch (p) {
		case 0:  return fmstore[m-'A'].w;
		case 1:  return fmstore[m-'A'].h;
		case 2:  return fmstore[m-'A'].w*fmstore[m-'A'].h;
		default: return 0;
		}
	}
	return 0;
}

int fimport(char m, char m2)
{
	int *arrayA;
	int i;
	
	arrayA=listarray(m2);

	
	if (m>='A' && m<='Z' && arrayA!=0)
	{
		int n=fmstore[m-'A'].h*fmstore[m-'A'].w;

		for (i=0; i<n; i++)
		{
			fmstore[m-'A'].fstore[i] = (float)listread(m2,i);
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
		int h1=fmstore[m-'A'].h;
		int w1=fmstore[m-'A'].w;
		int h2=fmstore[m2-'A'].h;
		int w2=fmstore[m2-'A'].w;

		for (i=0; i<h1; i++)
		{
			for (j=0; j<w1; j++)
			{
				float f=0.0;
				if (i>=h2 || j>=w2) 
					f=rd;
				else
					f=fmstore[m2-'A'].fstore[j+i*w2];

				fmstore[m-'A'].fstore[j+i*w1]=f;
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
	int i, h=fmstore[ln1-'A'].h;

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

	int h=fmstore[ln1-'A'].h;
	int w=fmstore[ln1-'A'].w;

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

	int h=fmstore[ln1-'A'].h;
	int w=fmstore[ln1-'A'].w;

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
	int h=fmstore[m-'A'].h;
	int w=fmstore[m-'A'].w;
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
	int h=fmstore[ln1-'A'].h;
	int w=fmstore[ln1-'A'].w;
	int mx,my;

#ifndef WIN32
	float tp[w*h]; //temp
#endif

	for (my=0; my<h; my++)
	{
		for (mx=0; mx<w; mx++)
		{
			tp[mx*h+my] = fget(ln1,mx,my);	
		}
	}

	if (lnx==ln1)
	{
		if (dbg) printf("transpose same array\n");
		fmstore[lnx-'A'].h = w;
		fmstore[lnx-'A'].w = h;		
	}
	else
	{
		//create	
		printf("create array %c\n",lnx);
		if (fmatrixcreate(lnx, h, w)<0) 
		{
			printf("Err: listcreate failed\n");
			return 1;
		}

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
 
	ha=(int)(fmstore[ln1-'A'].h);
	wa=(int)(fmstore[ln1-'A'].w);
	hb=(int)(fmstore[ln2-'A'].h);
	wb=(int)(fmstore[ln2-'A'].w);

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
 
	ha=(int)(fmstore[ln1-'A'].h);
	wa=(int)(fmstore[ln1-'A'].w);
	wb=(int)(fmstore[ln2-'A'].w);

#ifndef WIN32
	float tp[wb*ha]; //temp
#endif

	//if (wa != wb || ha != hb) 
	//{
	//	printf ("Bad Matrix size (%d,%d) + (%d,%d) \n",wa,ha,wb,hb);
	//	return 2; //bad array size
	//}

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

	int h=fmstore[ln1-'A'].h;
	int w=fmstore[ln1-'A'].w;
	int mx,my;
	//int szA=h*w;
	int szB=fmstore[ln2-'A'].w*fmstore[ln2-'A'].h;

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
	int h=fmstore[ln1-'A'].h;
	int w=fmstore[ln1-'A'].w;
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
