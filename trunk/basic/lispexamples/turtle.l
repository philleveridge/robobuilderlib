!DO

(DEF ROUND (X) 
	(INT (+ 0.5 X)))

(DEF CHT  (X Y T) 
	(AND (= X (ROUND (CAR T))) (= Y (ROUND (CAR (CDR T))))))

(DEF CHL  (X Y L) 
	(AND (= X (CAR L)) (= Y (CAR (CDR L)))))


(DEF move (T A D) 
	(WITH X Y R) 
	(SETQ (X Y R) T ) 
	(SETQ A (+ A R))
	(LIST ( + X (* D (COS A))) (+ Y (* D (SIN A))) A ))

(def foo (x) 
	(NTH (MOD (INT (/ (* X 4) (* 2 PI))) 4) '(> v < ^)))

(DEF PG ()      
	(FOR Y '(1 10) 
		(FOR X '(1 10) 
			(COND 	(CHT X Y TURTLE)        (PR (foo (nth 2 turtle)) ;) 
			      	(MEMBER (LIST X Y) LMS) (PR "#" ;)
				(PR '_ ;))
		) 
		(PR)
	))

(SETQ TURTLE (MOVE '(2 3 0.2) 0.1 4))
(SETQ LMS '( (2 3) (5 8)))
(PG)
(PR TURTLE)

;------------------------------------------

(def dist (x1 y1 x2 y2) (sqrt (+ (.^ (- x1 x2) 2) (.^ (- y1 y2) 2) )))

(SETQ landmarks  '(( 20.0 20.0) (80.0 80.0) (20.0 80.0) (80.0 20.0)))
(SETQ world_size 100.0)
(setq forward-noise 0.0 turn-noise 0.0  sense-noise 0.0)
(setq forward-noise 0.5 turn-noise 0.5  sense-noise 5.0)
(setq twopi (* 2 pi))


(def make-turtle () 
	(list (* (rnd) world_size) (* (rnd) world_size) (* (rnd) twopi)))

(def set-turtle  (x y o) 
	(list x y o))


(def move-turtle (T A D) 
	(WITH X Y R) 
	(SETQ 	(X Y R) T ) 
	(SETQ 	A (mod (plus A R  (rnd-g 0.0 turn-noise )) twopi))
	(SETQ 	D (+ D (rnd-g 0 forward-noise)))
	(SETQ 	X (mod (+ X (* D (COS A))) world_size ) 
		Y (mod (+ Y (* D (SIN A))) world_size ))
	(LIST X Y A ))

(def sense (t lm) 
	(setq (tx ty th) t dl '())
	(foreach lme lm 
		(setq (lmx lmy) lme)
	    	(setq dl (cons ( + (dist lmx lmy tx ty)  (rnd-g 0.0 sense-noise)) dl) )
	)
	(rev dl)
)

(def meas-prob (me t lmk)
	(setq prob 1.0 (tx ty) t)
	(foreach l lmk 
		(setq m1 (car me))
		(setq me (cdr me))
		(setq (lmx lmy) l)
		(setq prob (* prob (Gaus (dist lmx lmy tx ty) sense-noise m1)))
	)
	prob
)

; resampler
; ( W (turtle) )
(def resample (l  ts) 
      (SETQ R (RND)) 
      (foreach e l 
      (if (< r (/ (car e) ts)) 
            (return (cadr e)) 
            (setq r (- r (/ (car e) ts))))))


(def main (nsize step)
	(setq lot '() myrobot (make-turtle) z (sense myrobot landmarks ))
	(pr "myrobot=" myrobot ", z= " z)

	(setq forward-noise 0.05 turn-noise 0.05  sense-noise 5.0 ws 0.0 lot2 '())

	(for i (list 1 nsize)  
		(nconc lot (make-turtle)))

	(for j (list 1 step) 
		(setq 	myrobot (move-turtle myrobot 0.1 5.0) 
			z 	(sense myrobot landmarks ))

		(pr "step " j " myrobot=" myrobot ", z= " z)

		(foreach t lot 
			(setq  t    (move-turtle t 0.1 5.0)
			       mp   (meas-prob z t landmarks)
			       ws   (+ ws mp))
			(nconc lot2 (list mp t) )
		)

		; call resample
		(setq lot '())
		(for i (list 1 nsize) 
			(nconc lot  (resample lot2 ws) ))

		(pr "lot = " (len lot))
	)
)

(pr "type (main <num of particle> <number of steps>) i.e !> MAIN 100 2")




