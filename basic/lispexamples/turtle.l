!DO

(DEF ROUND (X) 
	(INT (+ 0.5 X)))

(DEF CHT  (X Y T) 
	(AND (= X (ROUND (CAR T))) (= Y (ROUND (CAR (CDR T))))))

(DEF CHL  (X Y L) 
	(AND (= X (CAR L)) (= Y (CAR (CDR L)))))

(setq forward-noise 0.0 turn-noise 0.0  sense-noise 0.0)

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


(def dist (x1 y1 x2 y2) (sqrt (+ (.^ (- x1 x2) 2) (.^ (- y1 y2) 2) )))

(SETQ landmarks  '(( 20.0 20.0) (80.0 80.0) (20.0 80.0) (80.0 20.0)))
(SETQ world_size 100.0)

(def sense (t lm) 
	(setq (tx ty th) t dl '())
	(foreach lme lm 
		(setq (lmx lmy) lme)
	    	(setq dl (cons ( + (dist lmx lmy tx ty)  (rnd-g 0.0 sense-noise)) dl) )
	)
	(rev dl)
)

;(def g2 (a b c ) (pr "a=" a " b=" b " c=" c " Gaus=" (gaus a b c)))

(def meas-prob (me t lmk)
	(setq prob 1.0)
	(setq (tx ty) t)
	(foreach l lmk 
		(setq m1 (car me))
		(setq me (cdr me))
		(setq (lmx lmy) l)
;		(pr tx "," ty "=>" lmx "," lmy "=" m1) (pr prob)
		(setq prob (* prob (Gaus (dist lmx lmy tx ty) sense-noise m1)))
	)
	prob
)
	
(setq turtle '(10 10 0.0))
(pr " t= " turtle)

(SETQ turtle (MOVE turtle (/ PI 2) 10.0))
(pr " t= " turtle)

(pr "sense = " (setq m (sense turtle landmarks)))

(setq forward-noise 0.5 turn-noise 0.5  sense-noise 5.0)

(pr "measure prob = " (meas-prob m turtle landmarks))

(def make-turtle () (list (* (rnd) world_size) (* (rnd) world_size) (* (rnd) (* 2 pi))))

(setq lot (list 
	(pr "mok turtle=" (make-turtle))
	(pr "mok turtle=" (make-turtle))
	(pr "mok turtle=" (make-turtle))))

(pr lot)

;(exit)



