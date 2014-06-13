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

#include "lists.h"
#include "mem.h"
#include "fmatrix.h"

//#include "express.h"
//#include "functions.h"
//#include "lists.h"

int abs(int);

int fm=0;
int fs=0;


/**********************************************************

Floating point Matrix library

**********************************************************/

fMatrix *newmatrix(int c, int r)
{
	fMatrix *n = (fMatrix *)bas_malloc(sizeof(fMatrix));
	int i;
	if (dbg) printf ("New Matrix %d,%d\n",c,r);
	n->h=r;
	n->w=c;
	n->fstore = 0;

	//allocate space

	n->fstore = (float *)bas_malloc(r*c*sizeof(float));

	if (n->fstore==0)
	{
		printf ("out of space  (%d)\n",fs);
	}
	else
	{
		for (i=0; i<r*c; i++) n->fstore[i]=0.0;
	}

	return n;
}

int delmatrix(fMatrix *m)
{
	if (m==0) return -1;
	if (dbg) printf ("Del Matrix [%d,%d]\n", m->w, m->h);

	if (m->h<=0 || m->w<=0 || m->fstore==0)
		return 0;

	m->h=0;
	m->w=0;	
	bas_free(m->fstore);
	bas_free(m);
	return 0;
}

fMatrix *fmatcp(fMatrix *A) // clone
{
	fMatrix *n;
	int w=A->w;
	int h=A->h;

	n = newmatrix(w, h);
	if (dbg) printf ("sz=%d\n",h*w);
	memcpy(n->fstore, A->fstore, sizeof(float)*h*w);
	return n;
}

fMatrix *fmatrshp(fMatrix *A, int c, int r) //clone &  resize
{
	fMatrix* n;
	int i, j;
	int w=A->w;
	int h=A->h;

	n = newmatrix(c, r);

  	for (i=0; i<r; i++)
		for (j=0; j<c; j++)
		{
			if (i<h && j<w)
				n->fstore[j+i*c]=A->fstore[j+i*w];
			else
				n->fstore[j+i*c]=0.0;
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
	int n=0;
	if (A != NULL)
	{
		printf ("Matrix %dx%d\n", A->w, A->h);
		n = fmatprint(stdout, A);
	}
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

fMatrix *fadd2(fMatrix *A, fMatrix *B, char op)  
{
	int rx,ry, ha,wa,wb,hb;

	fMatrix *R=NULL;

	wa=A->w;
	ha=A->h;
	wb=B->w;
	hb=B->h;

	if (dbg) printf ("matrix add (%c)\n",op);

	if (wa!=wb || ha!=hb)
	{
		if (dbg) printf ("warning size does not match (%d,%d) - (%d,%d)\n", wa,ha,wb,hb);
	}

	R = newmatrix(wa,ha);

	for (ry=0; ry<ha; ry++)
	{
		for (rx=0; rx<wa; rx++)
		{
			if (op=='+') R->fstore[rx+ry*wa] = fget2(A,rx,ry) + fget2(B,rx,ry);  
			if (op=='-') R->fstore[rx+ry*wa] = fget2(A,rx,ry) - fget2(B,rx,ry);  
			if (op=='/') 
			{
				float n= fget2(B,rx,ry);
				if (n != 0.0)
					R->fstore[rx+ry*wa] = fget2(A,rx,ry) / n;  
				else
					R->fstore[rx+ry*wa] = 0.0; //error
				
			}
			if (op=='*') R->fstore[rx+ry*wa] = fget2(A,rx,ry) * fget2(B,rx,ry);  
		}
	}
	return R;
}

fMatrix *ftranspose2(fMatrix *A)  
{
	int w=A->w;
	int h=A->h;
	int mx,my;
	fMatrix *R = newmatrix(h, w);;

	if (dbg) printf ("transpose %d,%d\n",w,h);

	for (my=0; my<h; my++)
	{
		for (mx=0; mx<w; mx++)
		{
			R->fstore[mx*h+my] = A->fstore[mx+my*w];
		}
	}
	return R;
}


fMatrix *fmultiply2(fMatrix *A,fMatrix *B)   
{
	int m,rx,ry, ha,wa,hb,wb;
	fMatrix *R; 

	wa=A->w;
	ha=A->h;
	wb=B->w;
	hb=B->h;

	if (dbg) printf (" Matrix mullt (%d,%d) * (%d,%d) \n",wa,ha,wb,hb);

	if (wa != hb) 
	{
		printf ("Bad Matrix size (%d,%d) * (%d,%d) \n",wa,ha,wb,hb);
		return NULL; //bad array size
	}

	R = newmatrix(wb, ha);

	for (ry=0; ry<ha; ry++)
	{
		for (rx=0; rx<wb; rx++)
		{
			R->fstore[rx+ry*wb] = 0.0f;
			for (m=0; m<hb; m++)
			{
				R->fstore[rx+ry*wb] += A->fstore[m+ry*wa] * B->fstore[rx+m*wb];
			}
		}
	}
	return R;
}

fMatrix *freplicate2(fMatrix *A, int m, int n)
{
	fMatrix *R;
 	int i,j,x,y;
	int w=A->w;
	int h=A->h;
	int wx=w*m;
	int hx=h*n;
	float *p,*pt;

	if (dbg) printf ("rep [%d,%d]\n", m,n);

	R = newmatrix(wx, hx);

	p =A->fstore;
	pt=R->fstore;

	for (j=0;j<h;j++)
		for (i=0;i<w;i++)
			for (x=0;x<m;x++)
				for (y=0;y<n;y++) 
				{
					pt[i+j*wx+y*wx+x*w] = p[i+j*w];
				}
	return R;
}


fMatrix *fmatsum2(fMatrix *A, int mode)   
{	
	fMatrix *R;
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
			fset2(R,0,my,p);
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
			fset2(R,mx,0,p);
		}
	}
	return R;
}

float fsum(fMatrix *A)
{
	float x=0.0;
	int i;
  	for (i=0; i < (A->h)*(A->w); i++)
		x += A->fstore[i];
	return x;
}

float fmatmax(fMatrix *A)
{
	float x=0.0;
	int i;
  	for (i=0; i < (A->h)*(A->w); i++)
		if (x < A->fstore[i]) x=A->fstore[i];
	return x;
}


/*
SETQ A [0 0 0 0 1 1 1 1 0 0 ]
SETQ K [-1 1]
CONV A K

CONV IM (GAUSK 7 1)
*/

fMatrix *fconvolve(fMatrix *A, fMatrix *B)  
{
	// A convoled with B, B is kernel	
	fMatrix *R, *t;
	int w=A->w;
	int h=A->h;
	int i,j,k,l;

	R = newmatrix(w, h);

	if (B->w > A->w ) { t=A; A=B; B=t; } //swap

	for (i=0; i<A->h; i++)
	{
		for (j=0; j<A->w; j++)
		{
			double p=0.0f;
			int wk=(B->w)/2;
			int hk=(B->h)/2;
			
			for (k=0; k<B->h; k++)
			{
				for (l=0; l<B->w; l++)
				{
//if (dbg) printf ("%d,%d - (%d,%d) %f %f %f\n",j,i,l,k, fget2(A,j+l-wk,i+k-hk), fget2(B,l,k), p);

					if (j+l-wk >=0 && j+l-wk < A->w && i+k-hk>=0 && i+k-hk < A->h)
					{


						p += (double)fget2(A,j+l-wk,i+k-hk)*(double)fget2(B,l,k);
					}
				}
			}
			fset2(R,j,i,(float)p);
		}
	}

	return R;
}

fMatrix   *fimport2(char m2, int c, int r)
{
	fMatrix *R;
	int i, j, n=0;
	R = newmatrix(c, r); 

	for (i=0; i<r; i++)
	{
		for (j=0; j<c; j++)
			fset2(R, j, i, (float)listread(m2,n++));
	}
	return R;
}	

fMatrix *fmatzerodiag2(fMatrix *A)   
{	
	int i, h=A->h;
	fMatrix *R = fmatcp(A); // clone
	for (i=0; i<h; i++)
	{
		fset2(R,i,i,0.0f);
	}
	return R;
}

fMatrix *fmatscale(fMatrix *A, float k)   
{	
	int i;
	fMatrix *R = fmatcp(A); // clone

	//printf ("[%d,%d] scale by %f\n", A->w,A->h,k);

	for (i=0; i<(A->h*A->w); i++)
	{
		R->fstore[i] = A->fstore[i]*k;
	}
	return R;
}

fMatrix *fmatflip(fMatrix *A)   
{	
	int i,j;
	fMatrix *R = fmatcp(A); // clone
	for (i=0; i<(A->h/2); i++)
	{
		for (j=0; j<A->w; j++)
		{
			R->fstore[j + i*A->w] = R->fstore[j + (A->h-i-1)*A->w];
		}
	}
	return R;
}

void fmatzero(fMatrix *A)   
{
	int i;
	//printf ("zero %d,%d\n", A->w,A->h);

	for (i=0; i<A->w*A->h; i++) 
		A->fstore[i]=0.0f;	
}

fMatrix *fmatzeroregion(fMatrix *A, int c1, int r1, int c2, int r2)   
{	
	fMatrix *R;
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
			fset2(R,j,i,0.0f);
		}
	}
	return R;
}


//Cholesky(self, ztol=1.0e-5)
//Computes the upper triangular Cholesky factorization of
//positive definite matrix.

fMatrix *Cholesky(fMatrix *s, float ztol)
{
/*
      	res = matrix([[]])
        res.zero(self.dimx, self.dimx)
*/
	int i,j,k=0;
	fMatrix *r= newmatrix(s->w, s->w);
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
			S += fget2(r,k,i) * fget2(r,k,i);
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
			fset2(r,i,i,0.0);
		}
		else
		{
			if (d<0.0)
			{
				printf("Matrix not positive-definite [%f]\n", d);
				return NULL; //bad array size
			}
			fset2(r,i,i,(float)sqrt(d));					
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
				S += fget2(r,k,i) * fget2(r,k,j);
			}
			if (fabs(S)<ztol)
				S=0.0;

			fset2(r,i,j, (fget2(s,i,j) - S)/fget2(r,i,i));
		}

	} 
	return r;
}

//CholeskyInverse(self)
//Computes inverse of matrix given its Cholesky upper Triangular
//decomposition of matrix.

fMatrix *CholeskyInverse(fMatrix *s)
{
	int i,j,k;
	fMatrix *r= newmatrix(s->w, s->w);

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
			S += fget2(s,j,k)*fget2(r,j,k);
		}
		fset2(r,j,j,1.0/(tjj*tjj) -S/tjj);

		for (i=j; i>=0; i--)
		{
			float r1 = 0.0;
			for (k=i+1; k<s->w; k++)
			{
				r1 += fget2(s,i,k)*fget2(r,k,j);
			}
			r1 = -r1/fget2(s,i,i);
			fset2(r,j,i, r1);
			fset2(r,i,j, r1);
		}
	}
	return r;
}

float detrminant(fMatrix *a, int k) 
{
 float s = 1, det = 0;
 fMatrix *b = newmatrix(k,k);
 
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
     fset2(b,i,j,0);
     if (i != 0 && j != c) {
      fset2(b,m,n,fget2(a,i,j));
      if (n < (k - 2))
       n++;
      else {
       n = 0;
       m++;
      }
     }
    }
   }
   det = det + s * (fget2(a,0,c) * detrminant(b, k - 1));
   s = -1 * s;
  }
 }
 return (det);
}
 
fMatrix *cofactors(fMatrix *num, int f) 
{
 fMatrix *b   = newmatrix(f,f);
 fMatrix *fac = newmatrix(f,f);

 int p, q, m, n, i, j;
 for (q = 0; q < f; q++) {
  for (p = 0; p < f; p++) {
   m = 0;
   n = 0;
   for (i = 0; i < f; i++) {
    for (j = 0; j < f; j++) {
     fset2(b,i,j, 0);
     if (i != q && j != p) {
      fset2(b,m,n,fget2(num,i,j));
      if (n < (f - 2))
       n++;
      else {
       n = 0;
       m++;
      }
     }
    }
   }
    fset2(fac,q,p,pow(-1, q + p) * detrminant(b, f - 1));
  }
 }
 return trans(num, fac, f);
}
 
fMatrix *trans(fMatrix *num, fMatrix *fac, int r)
{
 int i, j;
 fMatrix *b   = newmatrix(r,r);
 fMatrix *inv = newmatrix(r,r);
 float d;

 for (i = 0; i < r; i++) {
  for (j = 0; j < r; j++) {
	fset2(b,i,j, fget2(fac,j,i));
  }
 }
 
 d = detrminant(num, r);
 fset2(inv,i,j,0.0);
 for (i = 0; i < r; i++) {
  for (j = 0; j < r; j++) {
   fset2(inv,i,j,fget2(b,i,j)/d);
  }
 }
  return inv;
}

fMatrix *gausian(int Kernel_Size, int gaus_sigma)
{
	fMatrix *n = newmatrix(Kernel_Size, Kernel_Size);
	int i,j,k = (Kernel_Size-1)/2; 
	double Pi=3.14159265359	;
	double gs2=(double)(2*gaus_sigma*gaus_sigma);
	double gs3=(double)(gaus_sigma*gaus_sigma*gaus_sigma);
	double Tpigs3=2.0*Pi*gs3;

	for (i=1; i<=Kernel_Size; i++)
	{
		for (j=1; j<=Kernel_Size; j++) 
		{
			fset2(n, j-1, i-1, (float)( -( (j-k-1)/(Tpigs3) ) * exp ( - ( (i-k-1)*(i-k-1) + (j-k-1)*(j-k-1) )/ (gs2) )));
	    }
	}
	return n;
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

fMatrix *inverse(fMatrix *s)
{
	fMatrix *res=NULL;
	//printf("\nMATRIX inverse (%d,%d)\n",s->w,s->h);

	if (s->w==1 && s->h==1)
	{
		res = newmatrix(1,1);
		if (s->fstore[0]!=0.0)
			res->fstore[0] = 1/(s->fstore[0]);
		return res;
	}

	if (s->w==2 && s->h==2)
	{
		float det;
		res = newmatrix(2,2);
		det = s->fstore[0]* s->fstore[3] - s->fstore[1] * s->fstore[2];
		res->fstore[0]=s->fstore[3]/det;
		res->fstore[1]=-(s->fstore[1])/det;
		res->fstore[2]=-(s->fstore[2])/det;
		res->fstore[3]=s->fstore[0]/det;
		return res;
	}

	if (detrminant(s, s->w) ==  0)
		printf("\nMATRIX IS NOT INVERSIBLE\n");
	else
		return cofactors(s, s->w);
        return res;
}

void fmswap(fMatrix *list, int x, int y)
{
	int i;
	for (i=0; i<list->w; i++)
	{
		float temp1=fget2(list,i,x);
		float temp2=fget2(list,i,y);

		fset2(list,i,y,temp1);
		fset2(list,i,x,temp2);
	}
}

void fMatrixSort(fMatrix *list, int m, int n, int indx)
{
   int i,j,k;
   float key;

   if( m < n)
   {
      k = (m+n)/2;
      fmswap(list, m, k);
      key = fget2(list,indx,m);
      i = m+1;
      j = n;
      while(i <= j)
      {
         while((i <= n) && (fget2(list,indx,i) <= key))
                i++;
         while((j >= m) && (fget2(list,indx,j) > key))
                j--;
         if( i < j)
                fmswap(list,i,j);
      }
      // swap two elements
      fmswap(list,m,j);
      // recursively sort the lesser list
      fMatrixSort(list, m,   j-1, indx);
      fMatrixSort(list, j+1, n,   indx);
   }
}

fMatrix * fMatrixQSort(fMatrix *list, int indx)
{
	fMatrix *r=fmatcp(list);
	fMatrixSort(r,0, r->h-1, indx);
	return r;
}



