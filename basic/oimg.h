#ifndef OIMG_H
#define OIMG_H

typedef struct oimage {
	int w;
	int h;
	unsigned char *data;
} oImage;

extern oImage 	*makeimage	(int h, int w);
extern void 	delimage 	(oImage *ip);
extern oImage 	*cloneimage	(oImage *ip);
extern void 	printimage	(oImage *ip);

extern oImage 	*loadoImage	(char *name);
extern void 	imageogray	(oImage *im);
extern int 	image2Pgm	(oImage *image, char *fileName);

extern void 	clearoImage	(oImage *img, int c);
extern int 	maxval		(oImage *img);
extern void 	setpoint	(oImage *img, int x, int y, int c);
extern int 	getpoint	(oImage *img, int x, int y);
extern void 	drawrect	(oImage *img, int fx, int fy, int w,  int h,  int c);
extern void 	drawline	(oImage *img, int fx, int fy, int tx, int ty, int c);



#endif
