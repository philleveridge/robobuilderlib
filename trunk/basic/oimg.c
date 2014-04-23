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

extern int dbg;

extern int Height;
extern int Width;
extern int Depth;
extern int *frame;

extern unsigned char BMap[]; // max size

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

oImage *makeimage(int h, int w)
{
	if (dbg) {printf ("New Image\n");}
	oImage *p = (oImage *)bas_malloc(sizeof(oImage));
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
	if (dbg) {printf ("Clone Image\n");}
	oImage *nip = (oImage *)bas_malloc(sizeof(oImage));
	nip->h=ip->h;
	nip->w=ip->w;
	nip->data = (unsigned char *)bas_malloc(sizeof(unsigned char)*(ip->h)*(ip->w));	
	memcpy(nip->data, ip->data, sizeof(unsigned char)*(ip->w*ip->h));
	return nip;	
}

oImage *subimage(oImage *ip, int x, int y, int c, int r)
{
printf ("subimage - %d,%d [%d,%d]\n", x,y, c,r);

	oImage *nip = makeimage(r,c);

	for (int i=0; i<r; i++)
		for (int j=0; j<c; j++)
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
	if (tx<fx) tx=fx;
	if (ty<fy) ty=fy;

	for (int y=fy; y<=ty; y++)
		for (int x=fx; x<=tx; x++)
			setpoint(img,x,y,c); 

	return img;
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
	if (g>max) max=g;
	if (b>max) max=b;
	int min=r;
	if (g<min) min=g;
	if (b<min) min=b;
	return (max+min)/2;
}

#define RGBFN(a,b,c)  lumfn(a,b,c)

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

			*p++ = RGBFN(r, b, g);
		}
	}
     	return n;
}

oImage *FloadImage(char *name, oFilter n)
{
	unsigned char *img = BMap;

	if (loadJpg(name)>0)
		return NULL;

	oImage *im = makeimage(Height, Width);

	char *p = im->data;
	
	if (dbg) printf ("filtered : %s [%d,%d]\n", name, Height, Width);

	for (int i=0; i<Height; i++)
	{
		for (int j=0; j<Width; j++)
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
	unsigned char *img = &BMap[0];

	if (dbg) printf("threshold  [%d,%d,%d]\n" ,Height, Width, Depth);

	if (loadJpg(name)>0)
		return NULL;

	oImage *im = makeimage(nh, nw);

	char *p = im->data;

	for (int i=0; i<Height; i++)
	{
		int y=(i*nh)/Height;

		for (int j=0; j<Width; j++)
		{
			int x = (j*nw)/Width;

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

	if (dbg) imageshow(im);

	int num_runs = EncodeRuns2(im->data, im->h, im->w, max_runs); 

	if(num_runs == max_runs)
	{
		printf("WARNING! Exceeded maximum number of runs in EncodeRuns.\n");
	}

	if (dbg) show_run(num_runs);

	ConnectComponents(num_runs); 

	if (dbg) show_run(num_runs);

	int num_regions = ExtractRegions(max_regions, num_runs); 

	if (dbg) show_reg(num_regions);

	if(num_regions == max_regions)
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
			printf ("%c ", g1[(l-1)-(l-1)*(im->data[j+i*im->w])/mx]);
		}
		printf ("\n");
	}
}

void imageshow(oImage *im)
{
	for (int i=0; i<im->h; i++)
	{
		for (int j=0; j<im->w; j++)
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
	int mg=0;
	if (image==NULL) return 0;

	for (int i=0; i<image->h*image->w; i++) 
		if (image->data[i]>mg) mg=image->data[i];
	return mg;
}

int minval(oImage *image)
{
	int mg=255;
	if (image==NULL) return 0;

	for (int i=0; i<image->h*image->w; i++) 
		if (image->data[i]<mg) mg=image->data[i];
	return mg;
}

int sumoImage(oImage *image)
{
	int sm=0;
	if (image==NULL) return 0;

	for (int i=0; i<image->h*image->w; i++) 
		sm+=image->data[i];
	return sm;
}

oImage *threshoImage(oImage *image, int th)
{
	oImage *r = cloneimage(image);

	for (int i=0; i<image->h*image->w; i++) 
	{
		if (image->data[i]>th) 
			r->data[i]=255;
		else
			r->data[i]=0;
	}
	return r;
}

int image2Pgm(oImage *image, char *fileName)
{
	int i, mg=maxval(image);

	if (image==NULL) return -1;

	FILE *fp = fopen(fileName, "w");
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
	float n = (float)sumoImage(image);
	int sx=0, sy=0;
	for (int i=0; i<image->h; i++)
	{
		for (int j=0; j<image->w; j++)
		{
			sx += j*getpoint(image,j,i);	
			sy += i*getpoint(image,j,i);	
			//printf ("%d %d %d %d\n", j, i, sx, sy)	;	
		}
	}
	float fx= (float)sx/n;
	float fy= (float)sy/n;
	if (dbg) printf ("center = %d, %d -> %f, %f\n", sx, sy, fx, fy);

	if (rx != NULL)  *rx=(int)(fx+0.5);
	if (ry != NULL)  *ry=(int)(fy+0.5);

	return (int)(n+0.5);	
}

int meanshift(oImage *image, int noit, int wx, int wy, int *rx, int *ry)
{
	//assume center is center
	int cx=(image->w)/2;
	int cy=(image->h)/2;
	int m=0;
	for (int c=0; c<noit; c++)
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
		converging=1;
		//change wx and wy based on M0 
		//wx=sqrt(Mo/256)
		//wy=wx * 1.2
		//if abs(wx-oldwx)<th converging=1;
	}

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

		int c=0;
		for (int i=0; i<number_of_classes; i++)
		{
			for (int j=0; j<number_of_tuple; j++)
			{
				for (int k=0; k<(1<<tuple_size); k++)
				{
					store[c++]=0;
				}
			}
		}

		for (int i=0; i<number_of_classes; i++)
		{
			result[i]=0;
		}

		for (int i=0; i<tuple_size * number_of_tuple; i++)
		{
			connect[i]=i;
		}

		int number_of_swaps = (number_of_tuple*tuple_size)/2 ;
		double s = tuple_size * number_of_tuple;

		for (int i=0; i<number_of_swaps; i++)
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
	if ((n | 1) == 1)
	{
		printf ("Training data store\n");
		int t=0;
		for (int i=0; i<number_of_classes; i++)
		{
			for (int j=0; j<number_of_tuple; j++)
			{
				printf ("%d %d : ", i,j);
				for (int k=0; k<(1<<tuple_size); k++)
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
		for (int i=0; i<number_of_classes; i++)
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
	int r=0;
	if (im==NULL || iflag==0)
		return r;

	if (im->w != recog_w || im->h != recog_h)
	{
		printf ("error image size not same ! (%d,%d) - (%d,%d)\n", recog_w,recog_h, im->w, im->h);
		return 0;
	}

	int t=1<<tuple_size;
	int n=number_of_tuple*t;

	for (int class=0; class<number_of_classes; class++)
	{
		result[class]=0;
		for (int i=0; i< number_of_tuple; i++)
		{
			int v=0;
			for (int j=0; j<tuple_size; j++)
			{
                 		v = v << 1;
				if (im->data[connection_array(i,j)] > th ) 
					v = (v | 1);

			}
			if (store[class*n + i*t + v] ==1)
				result[class]++  ;
		}
	}  

	int c=0;
	for (int class=0; class<number_of_classes; class++)
	{ 
		if (result[class]>c) {c=result[class]; r=class;}
	}  
	return r;
}

void training (oImage *im, int  class)
{
	if (im==NULL || class<0 || class>=number_of_classes)
		return;

	if(iflag==0) recog_init(im->w, im->h, 2, 8);

	if (im->w != recog_w || im->h != recog_h)
	{
		printf ("error image size not same ! (%d,%d) - (%d,%d)\n", recog_w,recog_h, im->w, im->h);
		return;
	}

	int t=1<<tuple_size;
	int n=number_of_tuple*t;

	for (int i=0; i<number_of_tuple; i++)
	{
                int v=0;
           	for (int j=0; j<tuple_size; j++)
		{
                 	v = v << 1;
                 	if (im->data[connection_array(i,j)] > th ) 
			{
		  		v |= 1;
			}
		}		
		if (v>0) store[class*n + i*t + v] =1;
	}
}




