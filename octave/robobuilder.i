%module robobuilder
%{
extern "C" {
#include "robobuilder.h"
}
%}


%typemap(in) (const double* m, int r, int c) (Matrix mat) {
  mat = $input.matrix_value();
  $2 = mat.rows();
  $3 = mat.columns();
  $1 = (double *)malloc($2 * $3 * sizeof(double));
  for (int i = 0; i < $2; i++) {
    for (int j = 0; j < $3; j++) {
      $1[i*$3 + j] = mat(i,j);
    }
  }
}

%typemap(in) (servos_t *s) (Matrix mat) {
  mat = $input.matrix_value();
  $1 = (servos_t *)malloc(sizeof(servos_t));
  $1->nos = mat.columns(); 

  for (int j = 0; j < mat.columns(); j++) 
  {
      $1->values[j] = mat(j); 
  }
}

%typemap(out) (result_t *) (Matrix mat) {
    int32NDArray mat=int32NDArray(dim_vector(1,3));

    mat(0)=$1->x;
    mat(1)=$1->y;
    mat(2)=$1->z;
    free($1);
    $result=mat;
}

%typemap(out) (wck_t *) (Matrix mat) {
    int32NDArray mat=int32NDArray(dim_vector(1,2));

    mat(0)=$1->b1;
    mat(1)=$1->b2;
    free($1);
    $result=mat;
}

%typemap(out) (servos_t *) (Matrix mat) {
    int32NDArray mat=int32NDArray(dim_vector(1,$1->nos));

    for (int i=0; i<($1->nos); i++)
   	 mat(i)=$1->values[i];
    free($1);
    $result=mat;
}

extern int dbg;
extern char devicename[20];

void 		setdevname	(char *n);

void 		initserial	(int f);
int   		wckPosRead	(char ServoID);
int 		wckPassive	(int id);
int 		wckMovePos	(int id, int pos, int torq);
int  		readPSD		();   
int 		readIR		();
void 		standup 	(int n);
void 		setdh		(int n);
void 		PlayMotion	(int n);
void 		wckWriteIO	(unsigned char ServoID, unsigned char IO);

result_t	*readXYZ	();
wck_t   	*wckReadPos	(int, int);
wck_t 		*wckMovePos	(int id, int pos, int torq);
wck_t 		*wckPassive	(int id);

void 		SyncPosSend	(int SpeedLevel, servos_t *s);
void 		PlayPoseA	(int d, int f, int tq, int flag, servos_t *s);
servos_t 	*readservos	(int n);
