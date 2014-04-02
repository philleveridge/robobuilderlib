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

#include "mem.h"
#include "oimg.h"

extern int dbg;

extern int Height;
extern int Width;
extern int Depth;
extern int *frame;

extern unsigned char BMap[]; // max size

oImage *makeimage(int h, int w)
{
	if (dbg) {printf ("New Image\n");}
	oImage *p = (oImage *)bas_malloc(sizeof(oImage));
	p->h=h;
	p->w=w;
	p->data = (unsigned char *)bas_malloc(sizeof(unsigned char)*h*w);
	return p;	
}

void delimage (oImage *ip)
{

	if (ip != NULL)
	{
		if (dbg) {printf ("Delete Image %d,%d\n", ip->w, ip->h);}
		bas_free(ip->data);
		bas_free(ip);
	}
}

oImage *cloneimage(oImage *ip)
{
	if (dbg) {printf ("Clone Image\n");}
	oImage *nip = (oImage *)bas_malloc(sizeof(oImage));
	nip->h=ip->h;
	nip->w=ip->w;
	nip->data = (unsigned char *)bas_malloc(sizeof(unsigned char)*(ip->h)*(ip->w));
	for (int i=0; i<ip->h*ip->w; i++)
		nip->data[i] = ip->data[i];	
	return nip;	
}


oImage *loadoImage(char *name)
{
	unsigned char *img = BMap;


	if (loadJpg(name)>0)
		return NULL;

	oImage *n = makeimage(Height, Width);

	char *p = n->data;
	
	if (dbg) printf ("raw : %s [%d,%d]\n", name, Height, Width);

	for (int i=0; i<Height; i++)
	{
		for (int j=0; j<Width; j++)
		{
			unsigned char b=*img++;
			unsigned char g=*img++;
			unsigned char r=*img++;

			*p++ = (r + b + g)/3;
		}
	}
     	return n;
}

void imageogray(oImage *im)
{
	//char *g2 = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
       // int l=strlen(g2);

	char *g1 =  " .:-=+*#%@";
        int l=strlen(g1);

        int mx=0;
        for (int i=0;i<im->h*im->w; i++) 
	{
		if (im->data[i]>mx) mx=im->data[i];
	}

	for (int i=0; i<im->h; i++)
	{
		for (int j=0; j<im->w; j++)
		{
		     if (mx==0) 
			printf("- ");
		     else
		        //printf ("%c ", g2[(l*im->data[j+i*im->w])/mx]);
			printf ("%c ", g1[l-1-(im->data[j+i*im->w])/mx]);
		}
		printf ("\n");
	}
}

void printimage(oImage *ip)
{
	printf ("Image %d, %d\n", ip->w, ip->h);	
}



