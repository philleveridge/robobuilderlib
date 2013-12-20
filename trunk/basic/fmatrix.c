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
#endif


#include "express.h"
#include "functions.h"
#include "lists.h"

int abs(int);
extern int  dbg;

int fm=0;
int fs=0;


/**********************************************************

Floating point Matrix library

**********************************************************/

fMatrix newmatrix(int c, int r)
{
	fMatrix n;
	int i;
	if (dbg) printf ("New Matrix %d,%d\n",c,r);
	n.h=r;
	n.w=c;
	n.fstore = 0;

	//allocate space

	n.fstore = (float *)malloc(r*c*sizeof(float));

	if (n.fstore==0)
	{
		printf ("out of space  (%d)\n",fs);
	}
	else
	{
		for (i=0; i<r*c; i++) n.fstore[i]=0.0;
	}

	return n;
}

int delmatrix(fMatrix *m)
{
	if (dbg) printf ("Del Matrix [%d,%d]\n", m->w, m->h);

	if (m->h<=0 || m->w<=0 || m->fstore==0)
		return 0;

	m->h=0;
	m->w=0;	
	free(m->fstore);
	return 0;
}

fMatrix fmatcp(fMatrix *A) // clone
{
	fMatrix n;
	int i;
	int w=A->w;
	int h=A->h;

	n = newmatrix(w, h);

	if (dbg) printf ("sz=%d\n",h*w);

  	for (i=0; i<h*w; i++)
		n.fstore[i]=A->fstore[i];

	return n;
}

fMatrix fmatrshp(fMatrix *A, int c, int r) //clone &  resize
{
	fMatrix n;
	int i, j;
	int w=A->w;
	int h=A->h;

	n = newmatrix(c, r);

  	for (i=0; i<r; i++)
		for (j=0; j<c; j++)
		{
			if (i<h && j<w)
				n.fstore[j+i*c]=A->fstore[j+i*w];
			else
				n.fstore[j+i*c]=0.0;
		}

	return n;
}

int fmatprint(FILE *fp, fMatrix *A)
{
	int i,j;

	for (j=0; j<A->h;j++)
	{
		for (i=0; i<A->w; i++)
		{
			fprintf (fp, "%3f ", A->fstore[i+j*A->w]);
		}
		if (j+1<A->h) fprintf (fp, "\n");
	}
	return 0;
}

int fmatprint2(fMatrix *A)
{
	int n;
	printf ("Matrix %dx%d\n", A->w, A->h);
	n = fmatprint(stdout, A);
	return n;
}

float fget2(fMatrix *M, int c, int r)
{
	if (c>=0 && r>=0 && c<M->w && r<M->h && M->fstore!=0)
	{
		//printf ("ok- %d, %d %f\n", M->w, M->h, M->fstore[c+r*M->w]);
		return  M->fstore[c+r*(M->w)];
	}
	else
	{
		if (dbg) printf ("err- %d, %d\n", M->w, M->h);
		return 0.0;
	}
}

void fset2(fMatrix *M, int c, int r, float v)
{
	if (c>=0 && r>=0 && c<M->w && r<M->h && M->fstore!=0)
		M->fstore[c+r*M->w] =v;
}

fMatrix fadd2(fMatrix *A, fMatrix *B, char op)  
{
	int rx,ry, ha,wa,wb,hb;

	fMatrix R;
	R.h=0;
	R.w=0;

	wa=A->w;
	ha=A->h;
	wb=B->w;
	hb=B->h;

	if (dbg) printf ("matrix add (%c)\n",op);

	if (wa!=wb || ha!=hb)
	{
		if (dbg) printf ("size does not match (%d,%d) - (%d,%d)\n", wa,ha,wb,hb);
		//return R;
	}

	R = newmatrix(wa,ha);

	for (ry=0; ry<ha; ry++)
	{
		for (rx=0; rx<wa; rx++)
		{
			if (op=='+') R.fstore[rx+ry*wa] = fget2(A,rx,ry) + fget2(B,rx,ry);  
			if (op=='-') R.fstore[rx+ry*wa] = fget2(A,rx,ry) - fget2(B,rx,ry);  
			if (op=='/') 
			{
				float n= fget2(B,rx,ry);
				if (n != 0.0)
					R.fstore[rx+ry*wa] = fget2(A,rx,ry) / n;  
				else
					R.fstore[rx+ry*wa] = 0.0; //error
				
			}
			if (op=='*') R.fstore[rx+ry*wa] = fget2(A,rx,ry) * fget2(B,rx,ry);  
		}
	}
	return R;
}

fMatrix ftranspose2(fMatrix *A)  
{
	int w=A->w;
	int h=A->h;
	int mx,my;
	fMatrix R;

	if (dbg) printf ("transpose %d,%d\n",w,h);

	R = newmatrix(h, w);

	for (my=0; my<h; my++)
	{
		for (mx=0; mx<w; mx++)
		{
			R.fstore[mx*h+my] = A->fstore[mx+my*w];
		}
	}
	return R;
}


fMatrix fmultiply2(fMatrix *A,fMatrix *B)   
{
	int m,rx,ry, ha,wa,hb,wb;
	fMatrix R; 

	wa=A->w;
	ha=A->h;
	wb=B->w;
	hb=B->h;

	if (wa != hb) 
	{
		printf ("Bad Matrix size (%d,%d) * (%d,%d) \n",wa,ha,wb,hb);
		R.w=0; R.h=0; R.fstore=0;
		return R; //bad array size
	}

	R = newmatrix(wb, ha);

	for (ry=0; ry<ha; ry++)
	{
		for (rx=0; rx<wb; rx++)
		{
			R.fstore[rx+ry*wb] = 0.0f;
			for (m=0; m<hb; m++)
			{
				R.fstore[rx+ry*wb] += A->fstore[m+ry*wa] * B->fstore[rx+m*wb];
			}
		}
	}
	return R;
}

fMatrix freplicate2(fMatrix *A, int m, int n)
{
	fMatrix R;
 	int i,j,x,y;
	int w=A->w;
	int h=A->h;
	int wx=w*m;
	int hx=h*n;
	float *p,*pt;

	if (dbg) printf ("rep [%d,%d]\n", m,n);

	R = newmatrix(wx, hx);

	p =A->fstore;
	pt=R.fstore;

	for (j=0;j<h;j++)
		for (i=0;i<w;i++)
			for (x=0;x<m;x++)
				for (y=0;y<n;y++) 
				{
					pt[i+j*wx+y*wx+x*w] = p[i+j*w];
				}
	return R;
}


fMatrix fmatsum2(fMatrix *A, int mode)   
{	
	fMatrix R;
	int w=A->w;
	int h=A->h;
	int mx,my;
	float p=0.0;
	float t=0.0;

	if (mode==1 || mode==3)
	{
		R = newmatrix(1, h);  

		for (my=0;my<h; my++)
		{
			p=0.0;
			for (mx=0;mx<w; mx++)
			{
				t = fget2(A, mx,my);
				if (mode==3) t=t*t;
				p += t;
			}				
			fset2(&R,0,my,p);
		}
	}
	else if (mode==2 || mode==4)
	{
		R = newmatrix(w, 1);

		for (mx=0;mx<w; mx++)
		{
			p=0.0;
			for (my=0;my<h; my++)
			{
				t = fget2(A, mx,my);
				if (mode==4) t=t*t;
				p += t;
			}				
			fset2(&R,mx,0,p);
		}
	}
	return R;
}


fMatrix fconvolve2(fMatrix *A, fMatrix *B)  
{	
	fMatrix R;
	int w=A->w;
	int h=A->h;
	int mx,my;

	int szB=(B->w)*(B->h);	

	R = newmatrix(w, h);  

	if (szB==9) // 3x3 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				float p=0.0f;

				p += fget2(A,mx-1,my-1)*fget2(B,0,0);
				p += fget2(A,mx,my-1)  *fget2(B,1,0);
				p += fget2(A,mx+1,my-1)*fget2(B,2,0);

				p += fget2(A,mx-1,my)  *fget2(B,0,1);
				p += fget2(A,mx,my)    *fget2(B,1,1);
				p += fget2(A,mx+1,my)  *fget2(B,2,1);

				p += fget2(A,mx-1,my+1)*fget2(B,0,2);
				p += fget2(A,mx,my+1)  *fget2(B,1,2);
				p += fget2(A,mx+1,my+1)*fget2(B,2,2);

				fset2(&R,mx,my,p);
			}
		}
	}
	
	if (szB==2 && B->w==2 ) // 2x1 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				fset2(&R,mx,my, abs(fget2(A,mx,my)*fget2(B,0,0) 
					+ fget2(A,mx+1,my)*fget2(B,1,0)));
			}
		}
	}

	if (szB==2 && B->w==1 ) //  1x2 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				fset2(&R,mx,my,abs(fget2(A,mx,my)*fget2(B,0,0) + fget2(A,mx,my+1)*fget2(B,0,1)));
			}
		}
	}

	if (szB==4) // 2x2 kernel
	{
		for (my=0;my<h; my++)
		{
			for (mx=0;mx<w; mx++)
			{
				float p=0.0f;

				if (mx<w-1) 
					p += fget2(A,mx,my)*fget2(B,0,0) + fget2(A,mx+1,my)*fget2(B,1,0);

				if (my<h-1) 
					p += fget2(A,mx+1,my)*fget2(B,0,1) + fget2(A,mx+1,my+1)*fget2(B,1,1);

				fset2(&R,mx,my,p);
			}
		}
	}

	return R;	
}

fMatrix   fimport2(char m2, int c, int r)
{
	fMatrix R;
	int i, j, n=0;
	R = newmatrix(c, r); 

	for (i=0; i<r; i++)
	{
		for (j=0; j<c; j++)
			fset2(&R, j, i, (float)listread(m2,n++));
	}
	return R;
}	

fMatrix fmatzerodiag2(fMatrix *A)   
{	
	fMatrix R;
	int i, h=A->h;
	R = fmatcp(A); // clone
	for (i=0; i<h; i++)
	{
		fset2(&R,i,i,0.0f);
	}
	return R;
}

fMatrix fmatzeroregion(fMatrix *A, int c1, int r1, int c2, int r2)   
{	
	fMatrix R;
	int i, h=A->h;
	int j, w=A->w;
	R = fmatcp(A); // clone
	if (c1<0) c1=0;
	if (c2<0) c2=0;
	if (c1>c2) c1=c2;
	if (c1>=w) c1=w-1;
	if (c2>=w) c2=w-1;

	if (r1<0) r1=0;
	if (r2<0) r2=0;
	if (r1>r2) r1=r2;
	if (r1>=h) r1=h-1;
	if (r2>=h) r2=h-1;

	for (i=r1; i<r2; i++)
	{
		for (j=c1; j<c2; j++)
		{
			fset2(&R,j,i,0.0f);
		}
	}
	return R;
}


//Cholesky(self, ztol=1.0e-5)
//Computes the upper triangular Cholesky factorization of
//positive definite matrix.

fMatrix Cholesky(fMatrix *s, float ztol)
{
/*
      	res = matrix([[]])
        res.zero(self.dimx, self.dimx)
*/
	int i,j,k=0;
	fMatrix r= newmatrix(s->w, s->w);
/*       
        for i in range(self.dimx):
            S = sum([(res.value[k][i])**2 for k in range(i)])
            d = self.value[i][i] - S
*/
	for (i=0; i<s->w; i++)
	{		
		float d,S=0.0;
		for (k=0; k<i; k++)
		{
			S += fget2(&r,k,i) * fget2(&r,k,i);
		}
		d=fget2(s,i,i)-S;

/*
            if abs(d) < ztol:
                res.value[i][i] = 0.0
            else:
                if d < 0.0:
                    raise ValueError, """
                res.value[i][i] = sqrt(d)
*/
		if (fabs(d)<ztol)
		{
			fset2(&r,i,i,0.0);
		}
		else
		{
			if (d<0.0)
			{
				printf("Matrix not positive-definite [%f]\n", d);
				r.w=0; r.h=0; r.fstore=0;
				return r; //bad array size
			}
			fset2(&r,i,i,(float)sqrt(d));					
		}
/*
            for j in range(i+1, self.dimx):
                S = sum([res.value[k][i] * res.value[k][j] for k in range(self.dimx)])
                if abs(S) < ztol:
                    S = 0.0
                res.value[i][j] = (self.value[i][j] - S)/res.value[i][i]
*/

		for (j=i+1; j<s->w; j++)
		{
			S=0;
			for (k=0; k<s->w; k++)
			{
				S += fget2(&r,k,i) * fget2(&r,k,j);
			}
			if (fabs(S)<ztol)
				S=0.0;

			fset2(&r,i,j, (fget2(s,i,j) - S)/fget2(&r,i,i));
		}

	} 
	return r;
}

//CholeskyInverse(self)
//Computes inverse of matrix given its Cholesky upper Triangular
//decomposition of matrix.

fMatrix CholeskyInverse(fMatrix *s)
{
/*
        res = matrix([[]])
        res.zero(self.dimx, self.dimx)
*/
	int i,j,k;
	fMatrix r= newmatrix(s->w, s->w);

        // Backward step for inverse.
	
/*        
        for j in reversed(range(self.dimx)):
            tjj = self.value[j][j]
            S = sum([self.value[j][k]*res.value[j][k] for k in range(j+1, self.dimx)])
            res.value[j][j] = 1.0/tjj**2 - S/tjj
            for i in reversed(range(j)):
                res.value[j][i] = res.value[i][j] = -sum([self.value[i][k]*res.value[k][j] for k in range(i+1, self.dimx)])/self.value[i][i]
*/

	for (j=s->w-1; j>=0; j--)
	{
		float tjj=fget2(s,j,j);
		float S=0.0;
		for (k=j+1; k<s->w; k++)
		{
			S += fget2(s,j,k)*fget2(&r,j,k);
		}
		fset2(&r,j,j,1.0/(tjj*tjj) -S/tjj);

		for (i=j; i>=0; i--)
		{
			float r1 = 0.0;
			for (k=i+1; k<s->w; k++)
			{
				r1 += fget2(s,i,k)*fget2(&r,k,j);
			}
			r1 = -r1/fget2(s,i,i);
			fset2(&r,j,i, r1);
			fset2(&r,i,j, r1);
		}
	}

	return r;
}

fMatrix cofactors(fMatrix *num, int f) ;
fMatrix trans(fMatrix *num, fMatrix *fac, int r);


float detrminant(fMatrix *a, int k) 
{
 float s = 1, det = 0;
 fMatrix b = newmatrix(k,k);
 
 int i, j, m, n, c;
 if (k == 1) {
  return (fget2(a,0,0));
 } else {
  det = 0;
  for (c = 0; c < k; c++) {
   m = 0;
   n = 0;
   for (i = 0; i < k; i++) {
    for (j = 0; j < k; j++) {
     fset2(&b,i,j,0);
     if (i != 0 && j != c) {
      fset2(&b,m,n,fget2(a,i,j));
      if (n < (k - 2))
       n++;
      else {
       n = 0;
       m++;
      }
     }
    }
   }
   det = det + s * (fget2(a,0,c) * detrminant(&b, k - 1));
   s = -1 * s;
  }
 }
 return (det);
}
 
fMatrix cofactors(fMatrix *num, int f) 
{
 fMatrix b   = newmatrix(f,f);
 fMatrix fac = newmatrix(f,f);

 int p, q, m, n, i, j;
 for (q = 0; q < f; q++) {
  for (p = 0; p < f; p++) {
   m = 0;
   n = 0;
   for (i = 0; i < f; i++) {
    for (j = 0; j < f; j++) {
     fset2(&b,i,j, 0);
     if (i != q && j != p) {
      fset2(&b,m,n,fget2(num,i,j));
      if (n < (f - 2))
       n++;
      else {
       n = 0;
       m++;
      }
     }
    }
   }
    fset2(&fac,q,p,pow(-1, q + p) * detrminant(&b, f - 1));
  }
 }
 return trans(num, &fac, f);
}
 
fMatrix trans(fMatrix *num, fMatrix *fac, int r)
{
 int i, j;
 fMatrix b   = newmatrix(r,r);
 fMatrix inv = newmatrix(r,r);
 float d;

 for (i = 0; i < r; i++) {
  for (j = 0; j < r; j++) {
	fset2(&b,i,j, fget2(fac,j,i));
  }
 }
 
 d = detrminant(num, r);
 fset2(&inv,i,j,0.0);
 for (i = 0; i < r; i++) {
  for (j = 0; j < r; j++) {
   fset2(&inv,i,j,fget2(&b,i,j)/d);
  }
 }
  return inv;
}


/*
octave:1> i=[1 2;3 4]

   1   2
   3   4

octave:2> inverse(i)
  -2.00000   1.00000
   1.50000  -0.50000

i
!LET MT=[1.0 2.0;3.0 4.0]
!PRINT MT
!print INV(MT)
*/

fMatrix inverse(fMatrix *s)
{
	fMatrix res;
	res.w=0;
  	res.h=0;

	if (detrminant(s, s->w) ==  0)
		printf("\nMATRIX IS NOT INVERSIBLE\n");
	else
		return cofactors(s, s->w);
        return res;
}


