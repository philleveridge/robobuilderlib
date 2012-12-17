#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "win.h"
#endif

#ifdef LINUX
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "linux.h"

#define NULL (void *)0
#endif

#include "express.h"
#include "functions.h"
#include "lists.h"

extern int  dbg;

Matrix mstore[MAXLIST];

/**********************************************************/

void matzero(char ln1, int a, int b, int c, int d)   
{
	int *arrayA, szA, i, j;

	int h=mstore[ln1-'A'].h;
	int w=mstore[ln1-'A'].w;

	arrayA=listarray(ln1);
	szA   =listsize(ln1);

	if (arrayA==0 || szA==0 || a<0 || b<0 || c>=w || d>=h) 
	{
		printf ("Bad parameter zero @%c[%d,%d - %d,%d]\n",ln1,a,b,c,d);
		return; //bad array size
	}

	for (i=a; i<=c; i++)
	{
		for (j=b; j<=d; j++)
		{
			arrayA[j*w+i]=0;
		}
	}
	return;			
}

/******************************

A    = 1 2 3
       4 5 6

T(A) = 1 4
       2 5
       3 6

*******************************/


void transpose(char ln1)   // @A^T"
{
	int *arrayA, szA;
	int h=mstore[ln1-'A'].h;
	int w=mstore[ln1-'A'].w;

	arrayA=listarray(ln1);
	szA   =listsize(ln1);

	if (arrayA==0 || szA != h*w) 
		return; //error

	// Mresult = [w,h]
	int mx,my;
	int tp[w*h]; //temp
	for (my=0; my<h; my++)
	{
		for (mx=0; mx<w; mx++)
		{
			tp[mx*h+my] = arrayA[my*w+mx];	

//printf ("DBG: %d,%d = %d\n", mx,my, tp[mx*h+my]);
		}
	}

	for (mx=0; mx<h*w; mx++) arrayA[mx]=tp[mx];
		
	mstore[ln1-'A'].h = w;
	mstore[ln1-'A'].w = h;
	return;
}


/**********************************************************/

void multiply(char ln1, char ln2, char lnx)   // "@X = @A * @B"
{
	int *arrayA = listarray(ln1);
	int *arrayB = listarray(ln2);
	int *arrayX = listarray(lnx);

	int ha=mstore[ln1-'A'].h;
	int wa=mstore[ln1-'A'].w;
	int hb=mstore[ln2-'A'].h;
	int wb=mstore[ln2-'A'].w;
	int hx=mstore[lnx-'A'].h;
	int wx=mstore[lnx-'A'].w;

	// ha,wa * hb,wb
	// wa == hb
      	// Mresult =[ha*wb]

	if (arrayA==0 || arrayB==0 || arrayX==0) 
	{
		printf ("Bad Matrix definition\n");
		return; //error
	}
	if (hx != ha || wx != wb || wa != hb) 
	{
		printf ("Bad Matrix size (%d,%d) * (%d,%d) = (%d,%d)\n",ha,wa,hb,wb,hx,wx);
		return; //bad array size
	}

	int m,rx,ry;

	for (ry=0; ry<ha; ry++)
	{
		for (rx=0; rx<wb; rx++)
		{
			arrayX[rx+ry*wb] = 0;
			for (m=0; m<hb; m++)
			{
				// R(rx,ry) += A(rx,m) * B(m,ry)
				arrayX[rx+ry*wb] += (arrayA[rx*wa+m] * arrayB[ry+m*wb]);
			}
		}
	}
	return;
}


/**********************************************************/

void convolve(char ln1, char ln2)   // "@A (.) @B"
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
		return; //error
	if (szA<szB|| szA != h*w) 
		return; //bad array size

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
}


/**********************************************************/

void histogram(char ln1, int mode)   // "@A 
{
	int *arrayA, szA;

	int h=mstore[ln1-'A'].h;
	int w=mstore[ln1-'A'].w;
	int mx,my;

	arrayA=listarray(ln1);
	szA   =listsize(ln1);

	if (arrayA==0 || szA<=0 || h*w != szA) 
	{
		printf ("error in HIST\n");
		return; //bad array size
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
}
