#include <stdio.h>
#include <string.h>
#include <jpeglib.h>

int Height;
int Width;
int Depth;
int *frame;

typedef struct filter {
int minR; 
int minG;
int minB;
int maxR;
int maxG;
int maxB;
} Filter;


extern int dbg;

Filter n;

unsigned char BMap[640*480*3]; // max size
unsigned char * pTest;


int loadJpg(char* Name)
{
  unsigned char r,g,b;
  int width, height;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

  if (dbg) printf("read img\n");
  

  FILE * infile;        	/* source file */
  JSAMPARRAY pJpegBuffer;       /* Output row buffer */
  int row_stride;       	/* physical row width in output buffer */
  if ((infile = fopen(Name, "rb")) == NULL) 
  {
    fprintf(stderr, "can't open %s\n", Name);
    return 1;
  }

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);
  width = cinfo.output_width;
  height = cinfo.output_height;

  pTest=BMap;

  row_stride = width * cinfo.output_components ;
  pJpegBuffer = (*cinfo.mem->alloc_sarray)
    ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  int y=0;

  while (cinfo.output_scanline < cinfo.output_height) 
  {
    (void) jpeg_read_scanlines(&cinfo, pJpegBuffer, 1);
    
    for (int x=0;x<width;x++) {

       r = pJpegBuffer[0][cinfo.output_components*x];
       if (cinfo.output_components>2)
       {
           g = pJpegBuffer[0][cinfo.output_components*x+1];
           b = pJpegBuffer[0][cinfo.output_components*x+2];
       } else {
           g = r;
           b = r;
       }
     *(pTest++) = b;
     *(pTest++) = g;
     *(pTest++) = r;
    }
     y++;
  }

  fclose(infile);
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  Height = height;
  Width  = width;
  Depth  = 32;
  return 0;
}

int simplefilter(int mode, int sf)
{
     unsigned char *img = BMap;
	
     if (dbg) printf ("[%d,%d,%d]\n",Height, Width,Depth);

     for (int y=0; y<Height; y++)
     {
	 int j=(sf*y)/Height;
	 for (int x=0; x<Width; x++)
	 {
	     int i=(sf*x)/Width;

	     unsigned char b=*img++;
	     unsigned char g=*img++;
	     unsigned char r=*img++;
	
	     switch (mode) {
		case 0:
			frame[j*sf + i] += (r + b + g)/3;  // straight grey scale
			break;
		case 1:
			if ((r>=n.minR && r<=n.maxR) && (g>=n.minG && g<=n.maxG) && (b>=n.minB && b<=n.maxB))
			   frame[j*sf + i] += (r + b + g)/3;  
			break;
		case 2:
			break;
	     }

	     if (dbg==2) printf ("[%d,%d] = [%d,%d,%d]  (%d)=%d\n", x,y,r,g,b, j*sf + i,frame[j*sf + i] );
	}
     }
     return 0;
}

void output_grey2(int sz)
{
	char *g2 = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
        int l=strlen(g2);
        int mx=0;
        for (int i=0;i<sz*sz; i++) 
	{
		if (frame[i]>mx) mx=frame[i];
	}

	for (int i=0; i<sz; i++)
	{
		for (int j=0; j<sz; j++)
		{
		     printf ("%c ", g2[(l*frame[j+i*sz])/mx]);
		}
		printf ("\n");
	}
}

void output_grey1(int sz)
{
	char *g1 =  " .:-=+*#%@";
        int l=strlen(g1);
        int mx=0;
        for (int i=0;i<sz*sz; i++) 
	{
		if (frame[i]>mx) mx=frame[i];
	}

	for (int i=0; i<sz; i++)
	{
		for (int j=0; j<sz; j++)
		{
		     printf ("%c ", g1[l-1-(l*frame[j+i*sz])/mx]);
		}
		printf ("\n");
	}
}

void output_frame(int sz)
{
	for (int i=0; i<sz; i++)
	{
		for (int j=0; j<sz; j++)
		{
		     printf ("%3d ", frame[j+i*sz]);
		}
		printf ("\n");
	}
}

void output_cells(char * fn, int sz, int k)
{
	FILE *fp = fopen(fn,"w");
	if (fp==NULL) return;

	for (int i=0; i<sz; i++)
	{
		for (int j=0; j<sz; j++)
		{
		     fprintf (fp, "%d ", frame[j+i*sz]/k);
		}
		fprintf (fp, "\n");
	}
	fclose(fp);
}

void scale_array(int sz)
{
	int k= Height*Width/(sz*sz);

	for (int i=0; i<sz; i++)
	{
		for (int j=0; j<sz; j++)
		{
		     frame[j+i*sz] = frame[j+i*sz]/k;
		}
	}
}

void initframe(int x, int *f)
{
	for (int k=0; k<x*x; k++) f[k]=0;
        frame =&f[0];
}

//ie loadimage("test.jpeg", 16, data)
int loadimage(char *ifile, int sz, int *f)
{
	int x=sz;
	if (dbg) printf ("JPEG converter input %s [%d]\n", ifile, sz);

	initframe(x,f);

	if (loadJpg(ifile)!=0)
		return -1;

	simplefilter(0,x); // grey scale no filtering

	if (dbg) {
		output_grey1(x);
	}

	scale_array(x);
	return 0;
}

//ie filterimage("test.jpeg", scene, 16, 0, 0, 0, 255, 255, 255)
int filterimage(char *ifile, int *data, int x, int a, int b, int c, int d, int e, int f)
{
	n.minR = a;
	n.maxR = b;
	n.minG = b;
	n.maxG = d;
	n.minB = e;
	n.maxB = f;

	if (dbg)  {
		printf ("R [%d-%d] G [%d-%d] B[%d-%d]\n",n.minR, n.maxR,n.minG,  n.maxG, n.minB, n.maxB);
	}

	initframe(x,data);

	if (loadJpg(ifile)!=0)
		return -1;

	simplefilter(1,x);   // 1 colour filter

	if (dbg) {
		output_grey1(x);
	}

	scale_array(x);
	return 0;
}


