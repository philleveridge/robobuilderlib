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

#define NULL (void *)0
#endif

#ifdef AVR
#include "rprintf.h"
#endif



#include "express.h"
#include "functions.h"
#include "lists.h"

extern int  dbg;

Matrix mstore[26];

/**********************************************************/


int matzerodiag(char ln1)   
{
	int *arrayA, szA, i;

	int h=mstore[ln1-'A'].h;
	int w=mstore[ln1-'A'].w;

	arrayA=listarray(ln1);
	szA   =listsize(ln1);

	if (arrayA==0 || szA==0 || h != w) 
	{
		//printf ("Bad parameter zero @%c[%d,%d - %d,%d]\n",ln1,a,b,c,d);
		return 1; //bad array size
	}

	for (i=0; i<=h; i++)
	{
		arrayA[i*w+i]=0;
	}
	return 0;
}

int matzero(char ln1, int a, int b, int c, int d)   
{
	int *arrayA, szA, i, j;

	int h=mstore[ln1-'A'].h;
	int w=mstore[ln1-'A'].w;

	arrayA=listarray(ln1);
	szA   =listsize(ln1);

	if (arrayA==0 || szA==0 || a<0 || b<0 || c>=w || d>=h) 
	{
		//printf ("Bad parameter zero @%c[%d,%d - %d,%d]\n",ln1,a,b,c,d);
		return 1; //bad array size
	}

	for (i=a; i<=c; i++)
	{
		for (j=b; j<=d; j++)
		{
			arrayA[j*w+i]=0;
		}
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


int transpose(char ln1, char lnx)   // @A^T"
{
	int *arrayA, szA;
	int h=mstore[ln1-'A'].h;
	int w=mstore[ln1-'A'].w;
	int mx,my;

#ifndef WIN32
	int tp[w*h]; //temp
#endif

	arrayA=listarray(ln1);
	szA   =listsize(ln1);

	if (arrayA==0 || szA != h*w) 
		return 1; //error


	for (my=0; my<h; my++)
	{
		for (mx=0; mx<w; mx++)
		{
			tp[mx*h+my] = arrayA[my*w+mx];	
		}
	}

	if (lnx==ln1)
	{
		mstore[ln1-'A'].h = w;
		mstore[ln1-'A'].w = h;
		for (mx=0; mx<h*w; mx++) arrayA[mx]=tp[mx];		
	}
	else
	{
		//create
		int *arrayX;
		int r = listcreate(lnx, h*w, 2);

		if (r<0) 
		{
			rprintfProgStr(PSTR("Err: listcreate failed\n"));
			return 1;
		}

		mstore[lnx-'A'].w=h;
		mstore[lnx-'A'].h=w;
		mstore[lnx-'A'].name=lnx;
		arrayX = listarray(lnx);
		if (arrayX==(int *)0) 
			return 0;
		for (mx=0; mx<h*w; mx++) 
			arrayX[mx]=tp[mx];	
	}
	return 0;
}


/**********************************************************/

int multiply(char ln1, char ln2, char lnx)   // "@X = @A * @B"
{

	int m,rx,ry, ha,wa,hb,wb;
	int *arrayA, *arrayB, *arrayX;
 
	ha=(int)(mstore[ln1-'A'].h);
	wa=(int)(mstore[ln1-'A'].w);
	hb=(int)(mstore[ln2-'A'].h);
	wb=(int)(mstore[ln2-'A'].w);

	if (dbg) rprintf (" Matrix Mult %c(%d,%d) *  %c(%d,%d) \n",ln1,wa,ha,ln2,wb,hb);

	// ha,wa * hb,wb
	// wa == hb
      	// Mresult =[ha*wb]


	if (wa != hb) 
	{
		rprintf ("Bad Matrix size (%d,%d) * (%d,%d) \n",wa,ha,wb,hb);
		return 2; //bad array size
	}

	if (listcreate(lnx, wb*ha, 2)<0)
	{
			rprintfProgStr(PSTR("Can't create matrix\n"));
			return 1;
	}

	mstore[lnx-'A'].w=wb;
	mstore[lnx-'A'].h=ha;
	mstore[lnx-'A'].name=lnx;

	arrayA = listarray(ln1);
	arrayB = listarray(ln2);
	arrayX = listarray(lnx);

	if (arrayA==(int *)0 || arrayB==(int *)0 || arrayX==(int *)0) 
	{
		rprintfProgStr(PSTR("Bad Matrix definition\n"));
		return 1; //error
	}

	for (ry=0; ry<ha; ry++)
	{
		for (rx=0; rx<wb; rx++)
		{
			arrayX[rx+ry*wb] = 0;
			for (m=0; m<hb; m++)
			{
				// R(rx,ry) += A(ry,m) * B(m,rx)
				arrayX[rx+ry*wb] += (arrayA[ry*wa+m] * arrayB[rx+m*wb]);
			}
		}
	}
	return 0;
}


/**********************************************************/

int convolve(char ln1, char ln2)   // "@A (.) @B"
{	
	int *arrayA, szA;
	int *arrayB, szB;
	int h=mstore[ln1-'A'].h;
	int w=mstore[ln1-'A'].w;
	int mx,my;

	arrayA=listarray(ln1);
	szA   =listsize(ln1);
	arrayB=listarray(ln2);
	szB   =listsize(ln2);

	if (arrayA==0 || arrayB==0) 
		return 1 ; //error
	if (szA<szB|| szA != h*w) 
		return 2; //bad array size

	if (szB==9) // 3x3 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				int p=0;
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

				if (mx==0 || mx==w-1 || my==0 || my==h-1)
					scene[my*w+mx]=0;
				else
					scene[my*w+mx]=abs(p);
			}
		}
	}
	
	if (szB==2 && mstore[ln2-'A'].w==2 ) // 2x1 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				if (mx<w-1) 
					scene[my*w+mx]= abs(arrayA[my*w+mx]*arrayB[0] + arrayA[my*w+mx+1]*arrayB[1]);
				else
					scene[my*w+mx]= 0;
			}
		}
	}

	if (szB==2 && mstore[ln2-'A'].w==1 ) //  1x2 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				if (my<h-1) 
					scene[my*w+mx]= abs(arrayA[my*w+mx]*arrayB[0] + arrayA[(my+1)*w+mx]*arrayB[1]);
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

int histogram(char ln1, int mode)   // "@A 
{
	int *arrayA, szA;

	int h=mstore[ln1-'A'].h;
	int w=mstore[ln1-'A'].w;
	int mx,my;

	arrayA=listarray(ln1);
	szA   =listsize(ln1);

	if (arrayA==0 || szA<=0 || h*w != szA) 
	{
		//printf ("error in HIST\n");
		return 1; //bad array size
	}

//printf ("mode = %d, size=%d, mat=%c\n", mode, szA, ln1);

	if (mode==1)
	{
		for (my=0;my<h; my++)
		{
			int p=0;
			for (mx=0;mx<w; mx++)
			{
				p += arrayA[my*w+mx];	
			}				
			scene[my]=p;
		}
		nis=h;
	}
	else
	{
		for (mx=0;mx<w; mx++)
		{
			int p=0;
			for (my=0;my<h; my++)
			{
				p += arrayA[my*w+mx];	
			}				
			scene[mx]=p;
		}
		nis=w;
	}
	return 0;
}