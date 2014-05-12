#ifndef OIMG_H
#define OIMG_H

typedef struct filter {
	int minR; 
	int minG;
	int minB;
	int maxR;
	int maxG;
	int maxB;
} oFilter;

typedef struct
{
    int x;
    int y;
    int width;
    int height;
} MyRect;

typedef struct oimage {
	int w;
	int h;
	unsigned char *data;
} oImage;

extern oFilter 	setFilter	(int minR, int maxR, int minG, int maxG, int minB, int maxB);
extern void 	printFilter	(oFilter x);

extern oImage 	*makeimage	(int h, int w);
extern void 	delimage 	(oImage *ip);
extern oImage 	*cloneimage	(oImage *ip);
extern void 	printimage	(oImage *ip);
extern oImage 	*reshapeoImage	(oImage *image, int nw, int nh);
extern oImage 	*subimage	(oImage *ip, int x, int y, int c, int r);

extern oImage 	*loadoImage	(char *name);
extern oImage 	*FloadImage	(char *name, oFilter n);
extern oImage 	*cmapImage	(char *name, int nw, int nh) ;
extern oImage 	*threshoImage	(oImage *image, int th);
extern oImage 	*processImage	(char *fn, int nw, int nh);

extern void 	imageogray	(oImage *im);
extern int 	image2Pgm	(oImage *image, char *fileName);

extern void 	clearoImage	(oImage *img, int c);
extern int 	minval		(oImage *img);
extern int 	maxval		(oImage *img);
extern void 	setpoint	(oImage *img, int x, int y, int c);
extern int 	getpoint	(oImage *img, int x, int y);
extern void 	drawrect	(oImage *img, int fx, int fy, int w,  int h,  int c);
extern void 	drawline	(oImage *img, int fx, int fy, int tx, int ty, int c);
extern void 	imageshow	(oImage *im);

extern int 	sumoImage	(oImage *image);
extern int 	moment		(oImage *image, int *rx,  int *ry);
extern int 	meanshift	(oImage *image, int noit, int wx, int wy, int *rx, int *ry);
extern int 	camshift	(oImage *image, int noit, int wx, int wy, int *rx, int *ry, int *rw, int *rh);

extern void 	recog_init	(int w, int h, int nc, int ts);
extern void 	recog_del	();
extern int  	recognition 	(oImage *im );
extern void 	training	(oImage *im, int c);
extern int 	*result;
extern void 	recog_dump	(int n);
extern int 	recog_result	(int n);

#endif
