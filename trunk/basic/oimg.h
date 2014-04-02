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

#endif
