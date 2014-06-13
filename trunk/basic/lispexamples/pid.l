!do

;PID controller

(setq twopi (* 2 pi))

(def dist (x1 y1 x2 y2) (sqrt (+ (.^ (- x1 x2) 2) (.^ (- y1 y2) 2) )))

(SETQ landmarks  '(( 20.0 20.0) (80.0 80.0) (20.0 80.0) (80.0 20.0)))
(SETQ world_size 100.0)
(setq forward-noise 0.0 turn-noise 0.0  sense-noise 0.0)

(def turtle-print (t)  (pr "turtle [" (cell t 0 0) "," (cell t 1 0) "," (cell t 2 0)  "]" ))

(def turtle-set  (x y o) 
	(setq t [0 0 0])
	(setc t 0 0 x)
	(setc t 1 0 y)
	(setc t 2 0 o)
	t
)

(def turtle-make () 
	(turtle-set (* (rnd) world_size) (* (rnd) world_size) (* (rnd) twopi)))

(def turtle-get (t)
	(list (cell t 0 0) (cell t 1 0) (cell t 2 0)))


(def turtle-move (t d h)
	(setq tol 0.001 msa (/ pi 4.0) vlen 20.0)
	(setq drift (* pi (/ 10.0 180.0)))
	(if (> h msa)       (setq h msa))
	(if (< h (- 0 msa)) (setq h (- 0 msa)))
	(if (< d 0)         (setq d 0.0))
	(setq h (+ h drift))
	(setq turn (/ (* (tan h) d) vlen))

;	(SETQ 	h (mod (plus A R  (rnd-g 0.0 turn-noise )) twopi))
;	(SETQ 	D (+ D (rnd-g 0 forward-noise)))

	(setq (sx sy so) (turtle-get t))

	(if (< (abs turn) tol) 
		(do 
			(setc t 0 0 (+ sx (* d (cos so))))
			(setc t 1 0 (+ sy (* d (sin so))))
			(setc t 2 0 (mod (+ so turn) twopi))
		)
		(do 
			(setq rad (/ d turn))

			(setq cx (- sx (* rad (sin so))))
			(setq cy (+ sy (* rad (cos so))))
			(setq nh (mod (+ so turn) twopi))

			(setc t 0 0 (+ cx (* rad (sin nh))))
			(setc t 1 0 (- cy (* rad (cos nh))))
			(setc t 2 0 nh)
		)
	)
	t
)

(def turtle-sense (t lm) 
	(setq (tx ty th) (turtle-get t))
	(foreach lme lm 
		(setq (lmx lmy) lme)
	    	(setq dl (cons ( + (dist lmx lmy tx ty)  (rnd-g 0.0 sense-noise)) dl) )
	)
	(rev dl)
)

(def turtle-meas-prob (t me lmk)
	(setq prob 1.0 (tx ty th) (turtle-get t))
	(foreach l lmk 
		(setq m1 (car me))
		(setq me (cdr me))
		(setq (lmx lmy) l)
		(setq prob (* prob (Gaus (dist lmx lmy tx ty) sense-noise m1)))
	)
	prob
)

(setq lot ' lot2 '())

(def particles (nsize step)
	(setq 	lot 	(zero nsize 3)
		myrobot (turtle-make) 
		z 	(turtle-sense myrobot landmarks ))
	(pr "myrobot=";)
	(turtle-print myrobot)
	(pr "z= " z)
	(pr lot)

	(setq forward-noise 0.05 turn-noise 0.05  sense-noise 5.0 ws 0.0)

	(for j (list 1 step) 
		(setq 	myrobot (turtle-move  myrobot 0.1 5.0) 
			z 	(turtle-sense myrobot landmarks ))

		(pr "step " j " z= " z)
		(turtle-print myrobot)

		(for i (list 0 nsize)  (setc lot (turtle-make) i))

		(setq ml (zero nsize 4) lot2 (zero nsize 3))

		(for i  (list 0 (row lot)) 
			(setq  t    (turtle-move (row lot i) 5.0 0.1)
			       mp   (turtle-meas-prob t z landmarks)
			       ws   (+ ws mp))
			(setc ml   3 i mp)
			(setc lot2 t i )
		)
		(pr "sort=" (sort (+ (/ ml ws) lot2) 3))

		; call resample
		(setq lot '())
	)
)


; P D I controller

(def main (p-param d-param i-param )
	(setq turtle (turtle-set 0.0 1.0 0.0))
	(turtle-print turtle)
	(setq p1 (- 0 p-param) )
	(setq cte1 (cell turtle 1 0) sumcte 0.0)
	(for i '(1 100)
		(setq cte2 (cell turtle 1 0))
		(setq sumcte (+ sumcte cte2))
		(setq steer (- (- (* p1 cte2)  (* d-param (- cte2 cte1))) (* i-param sumcte)))
		(setq turtle (turtle-move turtle 1.0 steer))
		(setq cte1 cte2)
		(turtle-print turtle)
	)
	turtle
)

(main 0.2 3.0 0.004)

;(particles 200 1)




