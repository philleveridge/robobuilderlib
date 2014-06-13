#ifndef F_MATRIX_H
#define F_MATRIX_H

/**********************************************************

Floating point Matrix library

**********************************************************/

typedef struct fmat {
	int h; 
	int w;
	float *fstore;
} fMatrix;

extern fMatrix 	*newmatrix	(int c, int r);
extern int  	delmatrix	(fMatrix *m);
extern fMatrix 	*fmatcp		(fMatrix *A); 			// clone
extern fMatrix 	*fmatrshp	(fMatrix *A, int c, int r) ;	//clone &  resize

extern int 	fmatprint	(FILE *fp, fMatrix *A);
extern int 	fmatprint2	(fMatrix *A);

extern float 	fget2		(fMatrix *M, int c, int r);
extern void 	fset2		(fMatrix *M, int c, int r, float v);
extern void     fmatzero	(fMatrix *A) ;  

extern fMatrix *fadd2		(fMatrix *A, fMatrix *B, char op) ; 
extern fMatrix *ftranspose2	(fMatrix *A) ; 
extern fMatrix *fmultiply2	(fMatrix *A,fMatrix *B)   ;
extern fMatrix *freplicate2	(fMatrix *A, int m, int n);
extern fMatrix *fmatsum2	(fMatrix *A, int mode)  ; 
extern fMatrix *fconvolve	(fMatrix *A, fMatrix *B) ; 
extern fMatrix *fimport2	(char m2, int c, int r);
extern fMatrix *fmatzerodiag2	(fMatrix *A) ;
extern fMatrix *fmatzeroregion	(fMatrix *A, int c1, int r1, int c2, int r2);
extern fMatrix *Cholesky	(fMatrix *s, float ztol);
extern fMatrix *CholeskyInverse	(fMatrix *s);
extern fMatrix *cofactors	(fMatrix *num, int f) ;
extern fMatrix *trans		(fMatrix *num, fMatrix *fac, int r);
extern float   detrminant	(fMatrix *a, int k) ;
extern fMatrix *inverse(fMatrix *s);

extern fMatrix *fmatscale	(fMatrix *A, float k) ;  
extern fMatrix *fmatflip	(fMatrix *A); 
extern fMatrix *gausian		(int Kernel_Size, int gaus_sigma);

extern fMatrix *fMatrixQSort	(fMatrix *list, int indx);
extern float   fsum		(fMatrix *A);
extern float   fmatmax		(fMatrix *A);

#endif
