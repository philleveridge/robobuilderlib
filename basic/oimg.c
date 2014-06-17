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
#include "cmap.h"

//jpeg.c
unsigned char *loadJpg2 (char* Name, int *w, int *h);


oFilter setFilter(int minR, int maxR, int minG, int maxG, int minB, int maxB)
{
	oFilter x;
	x.minR=minR;
	x.minG=minG;
	x.minB=minB;
	x.maxR=maxR;
	x.maxG=maxG;
	x.maxB=maxB;
	return x;
}

void printFilter(oFilter x)
{
	printf ("[%d-%d, %d-%d, %d-%d]\n", x.minR, x.maxR, x.minG, x.maxG, x.minB, x.maxB);
}

oImage *makeimage(int h, int w)
{
	oImage *p;
	if (dbg) {printf ("New Image\n");}
	p = (oImage *)bas_malloc(sizeof(oImage));
	p->h=h;
	p->w=w;
	p->data = (unsigned char *)bas_malloc(sizeof(unsigned char)*h*w);
	memset(p->data, 0, sizeof(unsigned char)*h*w);
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
	oImage *nip;
	if (dbg) {printf ("Clone Image\n");}
	nip = (oImage *)bas_malloc(sizeof(oImage));
	nip->h=ip->h;
	nip->w=ip->w;
	nip->data = (unsigned char *)bas_malloc(sizeof(unsigned char)*(ip->h)*(ip->w));	
	memcpy(nip->data, ip->data, sizeof(unsigned char)*(ip->w*ip->h));
	return nip;	
}

oImage *subimage(oImage *ip, int x, int y, int c, int r)
{
	oImage *nip;
	int i,j;
	printf ("subimage - %d,%d [%d,%d]\n", x,y, c,r);

	nip = makeimage(r,c);

	for (i=0; i<r; i++)
		for (j=0; j<c; j++)
			setpoint(nip, j, i,  getpoint(ip, x+j, y+i));
	return nip;	
}

void setpoint(oImage *img, int x, int y, int c)
{
	if (img==NULL || (x<0) || (y<0) || (x>=img->w) || (y>=img->h)) return;
	img->data[x+y*img->w]=abs(c)%256;
}

int getpoint(oImage *img, int x, int y)
{
	if (img==NULL || (x<0) || (y<0) || (x>=img->w) || (y>=img->h)) 
		return 0;
	return img->data[x+y*img->w];
}

void drawline(oImage *img, int fx, int fy, int tx, int ty, int c)
{
       int x, y, t;
	   float grad;

       if (tx<fx) { t=tx; tx=fx; fx=t;}
       if (ty<fy) { t=ty; ty=fy; fy=t;}

       if (tx==fx)
       {
              for (y=fy; y<=ty; y++)
              {
                     setpoint(img,tx,y,c);
              }
              return;
       }
       if (ty==fy)
       {
              for (x=fx; x<=tx; x++)
              {
                     setpoint(img,x,ty,c);
              }
              return;
       }

       grad = (ty-fy) / (tx-fx) ;

       for (x=fx; x<=tx; x++)
       {
              y = fy + (grad * (x-fx) + 0.5);
              setpoint(img,x,y,c);
       }
       return;
}

void drawrect(oImage *img, int fx, int fy, int w, int h, int c)
{
	drawline(img, fx,  fy,  fx+w,  fy,   c);
	drawline(img, fx,  fy,  fx,    fy+h, c);
	drawline(img, fx,  fy+h,fx+w,  fy+h, c);
	drawline(img, fx+w,fy,  fx+w,  fy+h, c);
	return;
}

void clearoImage(oImage *img, int c)
{
	memset(img->data, c, sizeof(unsigned char)*img->h*img->w);
}

int avgfn(int r, int g, int b)
{
	return (r+g+b)/3;
}

int lumfn(int r, int g, int b)
{
	return (0.21*r+0.71*g+0.07*b)/3;
}

int lghtfn(int r, int g, int b)
{
	int max=r;
	int min=r;
	if (g>max) max=g;
	if (b>max) max=b;
	if (g<min) min=g;
	if (b<min) min=b;
	return (max+min)/2;
}

#define RGBFN(a,b,c)  lumfn(a,b,c)

oImage *loadoImage(char *name)
{
	int h,w,i,j;
	oImage *n;
	unsigned char *p;
	unsigned char *img = loadJpg2(name,&w,&h);
	if (img==NULL) return NULL;

	n = makeimage(h, w);
	p = n->data;
	
	if (dbg) printf ("raw : %s [%d,%d]\n", name, w, h);

	for (i=0; i<h; i++)
	{
		for (j=0; j<w; j++)
		{
			unsigned char b=*img++;
			unsigned char g=*img++;
			unsigned char r=*img++;

			*p++ = RGBFN(r, b, g);
		}
	}
     	return n;
}

oImage *FloadImage(char *name, oFilter n)
{
	int h,w,i,j;
	oImage *im;
	unsigned char *p;
	unsigned char *img = loadJpg2(name,&w,&h);
	if (img==NULL) return NULL;

	im = makeimage(h, w);
	p = im->data;
	
	if (dbg); printf ("filtered : %s [%d,%d]\n", name, w, h);
	if (dbg); printFilter(n);

	for (i=0; i<h; i++)
	{
		for (j=0; j<w; j++)
		{
			unsigned char b=*img++;
			unsigned char g=*img++;
			unsigned char r=*img++;

			if ((r>=n.minR && r<=n.maxR) && (g>=n.minG && g<=n.maxG) && (b>=n.minB && b<=n.maxB))
			   	*p = RGBFN(r, b, g);
			else
			 	*p = 0;
			p++;
		}
	}
     	return im;
}

oImage *cmapoImage(char *name, int nw, int nh) 
{
	oImage *im         = makeimage(nh, nw);
	unsigned char *p   = im->data;
	int h,w,i,j;
	unsigned char *img = loadJpg2(name,&w,&h);
	if (img==NULL) return NULL;

	if (dbg) printf("threshold  [%d,%d]\n" ,w, h);

	for (i=0; i<h; i++)
	{
		int y=(i*nh)/h;

		for (j=0; j<w; j++)
		{
			int x = (j*nw)/w;

			unsigned char b=*img++;
			unsigned char g=*img++;
			unsigned char r=*img++;

	     		 p[x+y*nw]  |=  testmap(r, g, b);
		}
	}
	return im;
}

//tbd - processImage not complete
oImage *processImage(char *fn, int nw, int nh)
{
	oImage *im = cmapoImage(fn, nw, nh); 
	int num_runs, num_regions, max_area;
	if (dbg) imageshow(im);

	num_runs = EncodeRuns2(im->data, im->h, im->w, MAX_RUNS); 

	if(num_runs >= MAX_RUNS)
	{
		printf("WARNING! Exceeded maximum number of runs in EncodeRuns.\n");
	}

	if (dbg) show_run(num_runs);

	ConnectComponents(num_runs); 

	if (dbg) show_run(num_runs);

	num_regions = ExtractRegions(MAX_REGIONS, num_runs); 

	if (dbg) show_reg(num_regions);

	if(num_regions >= MAX_REGIONS)
	{
		printf("WARNING! Exceeded maximum number of regions in ExtractRegions.\n");
	}

	max_area = SeparateRegions(&color[0], no_colours(), num_regions); 
	SortRegions(&color[0], no_colours(), max_area); 

	return im;
}

void imageogray(oImage *im)
{
	//char *g2 = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
       // int l=strlen(g2);

	char *g1 =  " .:-=+*#%@";
    int l=strlen(g1);

    int i, j, mx=0;
    for (i=0;i<im->h*im->w; i++) 
	{
		if (im->data[i]>mx) mx=im->data[i];
	}

	for (i=0; i<im->h; i++)
	{
		for (j=0; j<im->w; j++)
		{
		     if (mx==0) 
			printf("- ");
		     else
		        //printf ("%c ", g2[(l*im->data[j+i*im->w])/mx]);
			printf ("%c ", g1[(l-1)-(l-1)*(im->data[j+i*im->w])/mx]);
		}
		printf ("\n");
	}
}

void imageshow(oImage *im)
{
	int i,j;
	for (i=0; i<im->h; i++)
	{
		for (j=0; j<im->w; j++)
		{
			printf ("%3d ", im->data[j+i*im->w]);
		}
		printf ("\n");
	}
}

void printimage(oImage *ip)
{
	printf ("Image %d, %d", ip->w, ip->h);	
}

int maxval(oImage *image)
{
	int i, mg=0;
	if (image==NULL) return 0;

	for (i=0; i<image->h*image->w; i++) 
		if (image->data[i]>mg) mg=image->data[i];
	return mg;
}

int minval(oImage *image)
{
	int i, mg=255;
	if (image==NULL) return 0;

	for (i=0; i<image->h*image->w; i++) 
		if (image->data[i]<mg) mg=image->data[i];
	return mg;
}

int sumoImage(oImage *image)
{
	int i, sm=0;
	if (image==NULL) return 0;

	for (i=0; i<image->h*image->w; i++) 
		sm+=image->data[i];
	return sm;
}

oImage *threshoImage(oImage *image, int th1, int th2)
{
	oImage *r = cloneimage(image);
	int i;
	for (i=0; i<image->h*image->w; i++) 
	{
		if (image->data[i]>=th2) 
			r->data[i]=1;
		if (image->data[i]<th1) 
			r->data[i]=0;
	}
	return r;
}

int image2Pgm(oImage *image, char *fileName)
{
	int i, mg=maxval(image);
	FILE *fp;
	if (image==NULL) return -1;

	fp = fopen(fileName, "w");
	if (!fp)
	{
		printf("No such file %s\n", fileName);
		return -1;
	}

	fprintf (fp,"P5\n%d %d\n%d\n", image->w, image->h, mg);

	for (i = 0; i < (image->w * image->h); i++)
	{
		fputc(image->data[i], fp);
	}
	fclose(fp);
	return 0;
}

oImage *reshapeoImage(oImage *image, int nw, int nh)
{
	return subimage(image, 0, 0, nh, nw);
}

int moment(oImage *image, int *rx, int *ry)
{
	int i,j;
	float n = (float)sumoImage(image);
	float sx=0.0, sy=0.0;
	if (n==0.0) 
	{
		*rx=image->w/2;
		*ry=image->h/2;	
		return 0;	
	}

	for (i=0; i<image->h; i++)
	{
		for (j=0; j<image->w; j++)
		{
			sx += (float)(j*getpoint(image,j,i));	
			sy += (float)(i*getpoint(image,j,i));	
		}
	}

	if (rx != NULL)  *rx=(int)(sx/n+0.5);
	if (ry != NULL)  *ry=(int)(sy/n+0.5);

	return (int)(n+0.5);	
}

int meanshift(oImage *image, int noit, int wx, int wy, int *rx, int *ry)
{
	//assume center is center
	int cx=(image->w)/2;
	int cy=(image->h)/2;
	int c, m=0;
	for (c=0; c<noit; c++)
	{
		int   ncx, ncy;
		oImage *sw = subimage(image, cx-wx/2, cy-wy/2, wy, wx);	
if (dbg) imageshow(sw);
		m = moment(sw, &ncx, &ncy);
		ncx += cx-wx/2; //(image->w)/2;
		ncy += cy-wy/2; //(image->h)/2;
		delimage(sw);
		printf ("%d) %d %d\n",c, ncx,ncy);
		if (cx==ncx && cy==ncy) break;
		cx=ncx;
		cy=ncy;
	}
	*rx=cx;
	*ry=cy;
	return m;
}

int camshift(oImage *image, int noit, int wx, int wy, int *rx, int *ry, int *rw, int *rh)
{
/*
The Camshift algorithm, Continuously Adaptive Mean Shift , that encapsulates the Mean-Shift in a loop by varying the size of the window until convergence. At each iteration, the mean shift is applied with a window of a given size. After convergence of the mean shift, the procedure is re-iterated with a window centered on the position found by the mean shift, but depending on the size of the zero-order moment of the spatial distribution of the probability of skin color previously calculated by the mean shift.

The Camshift applies to the segmentation of still images after convergence of the mean shift, the height of the selected window is 20% larger than its width, but this choice is arbitrary. It is suitable in cases of segmenting a face relatively frontal pose. In other words, the aspect ratio is given as an example, but it can be changed to a different orientation of the face or any other type of object. The various steps of Camshift are:

Initialize W  : position and size.
While W is moved more than a certain threshold and the maximum number of iterations has not been reached:
apply the mean shift: remember the moment and zero-order
center W on and assign the width and height
*/
	int cx, cy, nw, nh;
	int converging = 0;
	while (!converging)
	{
		int m = meanshift(image, noit, wx, wy, rx, ry);
		cx=*rx;
		cy=*ry;
		//change wx and wy based on M0 
		//wx=sqrt(Mo/256)
		m=sqrt(m/256);
		nw=wx;
		//wy=wx * 1.2
		nh=(float)nw*1.2;
		//if abs(wx-oldwx)<th converging=1;
		converging=1;
	}
	*rx=cx;
	*ry=cy;
	*rw=nw;
	*rh=nh;
	return 0;
}

/*
tuple_size  i.e. 8
height width arraysize =(height*width)
number_of_tuples = arraysize/tuple_size
connection_array (number_of_tuples, tuple_size)
store    (number_of_class, number of tuple, 2^tuple_size)

train  class=1
1 0 0       	4,5,7
1 0 1		
1 1 1		

1 0 0           4,5,6
1 0 1
1 1 0  

1 0 1           5,5,6 
1 0 1
1 1 0

------
recognise
1 0 1           5,6,6 -> 1 + 0 + 1 = 2/3 = 66% (class 1)
1 1 0
1 1 0
*/

int th			= 127;
int tuple_size		= 8;
int number_of_tuple	= 10;
int number_of_classes	= 2;
int recog_h		= 0;
int recog_w		= 0;
int *store 		= NULL;		//[number_of_classes] [number_of_tuple] [1<<tuple_size]
int *result 		= NULL;		//[number_of_classes]
int *connect 		= NULL;		//[tuple_size * number_of_tuples]
int iflag		= 0;

void recog_del()
{
	if (iflag)
	{
		if (dbg); printf("Recogniser clear\n");

		if (store  !=NULL) bas_free(store);
		if (result !=NULL) bas_free(result);
		if (connect!=NULL) bas_free(connect);
		recog_h	= 0;
		recog_w	= 0;
		iflag	= 0;
	}
}

void recog_init(int w, int h, int nc, int ts)
{
	if (iflag==0)
	{
		int i,j,k,c=0;
		int number_of_swaps;
		double s;
		if (dbg); printf("Recogniser init (%d,%d) %d %d\n",w,h,nc,ts);
		iflag=1;

		tuple_size		= ts;
		number_of_classes	= nc;
		number_of_tuple		= (h*w)/ts;
		recog_h			= w;
		recog_w			= h;

		if (store  ==NULL) store  =(int *)bas_malloc(sizeof(int)*number_of_classes*number_of_tuple*(1<<tuple_size));
		if (result ==NULL) result =(int *)bas_malloc(sizeof(int)*number_of_classes);
		if (connect==NULL) connect=(int *)bas_malloc(sizeof(int)*number_of_tuple*tuple_size);


		for (i=0; i<number_of_classes; i++)
		{
			for (j=0; j<number_of_tuple; j++)
			{
				for (k=0; k<(1<<tuple_size); k++)
				{
					store[c++]=0;
				}
			}
		}

		for (i=0; i<number_of_classes; i++)
		{
			result[i]=0;
		}

		for (i=0; i<tuple_size * number_of_tuple; i++)
		{
			connect[i]=i;
		}

		number_of_swaps = (number_of_tuple*tuple_size)/2 ;
		s = tuple_size * number_of_tuple;

		for (i=0; i<number_of_swaps; i++)
		{
			double r1 = (double)rand()/(double)RAND_MAX;
			double r2 = (double)rand()/(double)RAND_MAX;
			int p1=(int)(r1*s);
			int p2=(int)(r2*s);
			int t=connect[p1];
			connect[p1]=connect[p2];
			connect[p2]=t;
		}

		//for (int i=0; i<tuple_size * number_of_tuple; i++)
		//{
		//	printf ("%d %d\n",i, connect[i]);
		//}
	}	
}

void recog_dump(int n)
{
	int i,j,k,t=0;
	if ((n | 1) == 1)
	{
		printf ("Training data store\n");

		for (i=0; i<number_of_classes; i++)
		{
			for (j=0; j<number_of_tuple; j++)
			{
				printf ("%d %d : ", i,j);
				for (k=0; k<(1<<tuple_size); k++)
				{
					if (store[t++]==1) printf ("%d,", k);
				}
				printf ("\n");
			}
		}
	}

	if ((n | 2) == 2)
	{
		printf ("\nRecogniser result data\n");
		for (i=0; i<number_of_classes; i++)
		{
			printf ("%d %d\n", i, result[i]);
		}
	}	
}

int connection_array(int i, int j)
{
	return connect[i*tuple_size+j];
}

int recog_result(int n)
{
	return  (n<0 || n>=number_of_classes)?-1:result[n];
}

int  recognition (oImage *im)
{
	int clss, c=0, r=0;
	int i, j, t, n;

	if (im==NULL || iflag==0)
		return r;

	if (im->w != recog_w || im->h != recog_h)
	{
		printf ("error image size not same ! (%d,%d) - (%d,%d)\n", recog_w,recog_h, im->w, im->h);
		return 0;
	}

	t=1<<tuple_size;
	n=number_of_tuple*t;

	for (clss=0; clss<number_of_classes; clss++)
	{
		result[clss]=0;
		for (i=0; i< number_of_tuple; i++)
		{
			int v=0;
			for (j=0; j<tuple_size; j++)
			{
                 		v = v << 1;
				if (im->data[connection_array(i,j)] > th ) 
					v = (v | 1);

			}
			if (store[clss*n + i*t + v] ==1)
				result[clss]++  ;
		}
	}  

	for (clss=0; clss<number_of_classes; clss++)
	{ 
		if (result[clss]>c) {c=result[clss]; r=clss;}
	}  
	return r;
}

void training (oImage *im, int clss)
{
		int t, n, i, j;

	if (im==NULL || clss<0 || clss>=number_of_classes)
		return;

	if(iflag==0) recog_init(im->w, im->h, 2, 8);

	if (im->w != recog_w || im->h != recog_h)
	{
		printf ("error image size not same ! (%d,%d) - (%d,%d)\n", recog_w,recog_h, im->w, im->h);
		return;
	}

	t=1<<tuple_size;
	n=number_of_tuple*t;

	for (i=0; i<number_of_tuple; i++)
	{
        int v=0;
        for (j=0; j<tuple_size; j++)
		{
            v = v << 1;
            if (im->data[connection_array(i,j)] > th ) 
			{
		  		v |= 1;
			}
		}		
		if (v>0) store[clss*n + i*t + v] =1;
	}
}




