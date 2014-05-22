!do

; (setq x 8 m 10 s2 4) (pr "x=" x " gaus=" (gaus m s2 x))

; kalman filter
; code here

(def update (m1 v1 m2 v2)
   (list (/ (+ (* v2 m1) (* v1 m2)) (+ v1 v2)) 
         (/ 1 (+ (/ 1 v1) (/ 1 v2)))))

(def predict (m1 v1 m2 v2) (list (+ m1 m2) (+ v1 v2)))

;(pr "predict = " (predict 10.0 4.0 12.0 4.0))
;(pr "Update = " (update 10.0 8.0 13.0 2.0))

(setq 	meas 	'(5.0 6.0 7.0 9.0 10.0)
	mot  	'(1.0 1.0 2.0 1.0 1.0)
 	mes2 	4.0 
	mos2 	2.0 
	mu 	0.0 
	sig 	10000)

(while (not (null meas)) 
	(setq (mu sig) (update mu sig (car meas) mes2))
	(setq meas (cdr meas))
	(pr "Update  : mu=" mu ", sig=" sig)

	(setq (mu sig) (predict mu sig (car mot) mos2))
	(setq mot (cdr mot))
	(pr "Predict : mu=" mu ", sig=" sig)
)

(exit)
