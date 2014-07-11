#ifndef O_PART_H
#define O_PART_H

#include "oobj.h"
#include "fmatrix.h"

typedef struct Turtles   {
	float x;
	float y;
	float orientation;

	float length;
	float steering_noise;
	float distance_noise;
	float measurement_noise;
	float drift;
	float world_size;

	int   num_collisions;
	int   num_steps;
} tTurtle, *tTurtlep;


typedef struct Particles {
	int N;
	float steering_noise;
	float distance_noise;
	float measurement_noise;
	float world_size;
	tTurtlep *turtle_data;
} tParticle, *tParticlep;

/**********************************************************/
/*  Plans                                                 */
/**********************************************************/

extern fMatrix *	Astar			(fMatrix *grid, int sx, int sy, int gx, int gy);
extern fMatrix *	smooth			(fMatrix *m, float weight_data, float weight_smooth, float tolerance);
extern void 		plan_print		(fMatrix *grid, fMatrix *plan);

/**********************************************************/
/*  turtles                                               */
/**********************************************************/

extern double 		rgauss			(double mean, double variance);
extern void 		set_params		(float new_s_noise, float new_d_noise, float new_m_noise, float new_length, float new_drift) ;

extern tTurtlep 	turtle_make		(float world_size, float x, float y, float h) ;
extern tTurtlep 	turtle_make_random	(float world_size);
extern tTurtlep 	turtle_clone		(tTurtlep p) ;
extern void 		turtle_del		(tTurtlep p) ;

extern void 		turtle_print		(tTurtlep p) ;
extern void 		turtle_set		(tTurtlep p, float a, float b, float c) ;
extern void 		turtle_set_noise	(tTurtlep p, float new_s_noise, float new_d_noise, float new_m_noise) ;
extern void 		turtle_set_drift	(tTurtlep p, float a) ;

extern tTurtlep 	turtle_move		(tTurtlep o, float steering, float distance) ;
extern tTurtlep 	turtle_simple_move	(tTurtlep o, float steering, float distance) ;
extern fMatrix	*	turtle_sense		(tTurtlep o, fMatrix *landmarks) ;
extern void 		turtle_position		(tTurtlep o, float *xp, float *yp, float *op) ;
extern double 		turtle_measure_prob	(tTurtlep p, float   measurement_x, float   measurement_y) ;
extern double 		turtle_measure_prob2	(tTurtlep p, fMatrix *measurement,  fMatrix *landmarks) ;

extern int 		turtle_check_collision	(tTurtlep p, fMatrix *grid) ;
extern int 		turtle_check_goal	(tTurtlep p, float goal_x, float goal_y, float threshold) ;


/**********************************************************/
/*  particles                                             */
/**********************************************************/

extern tParticlep 	particles_make		(int N, float worldsize) ;
extern tParticlep 	particles_make2		(int N, float x, float y, float h, float worldsize);
extern tParticlep 	particles_make_random	(int N, float worldsize);
extern tParticlep	particles_clone		(tParticlep p) ;
extern void 		particles_del		(tParticlep p) ;

extern void 		particles_print		(tParticlep p) ;
extern tParticlep	particles_sense		(tParticlep p, fMatrix *Z) ;
extern tParticlep	particles_move		(tParticlep p, double steer, double speed, fMatrix *grid, int t ) ;	
extern void 		particles_get_position	(tParticlep p, float *xp, float *yp, float *op) ;

#endif
