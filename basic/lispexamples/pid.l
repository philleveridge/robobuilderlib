!do

;PID controller

(setq twopi (* 2 pi))

(def tprint (t)  (pr "turtle [" (cell t 0 0) "," (cell t 1 0) "," (cell t 2 0)  "]"))

(def move (t d h)
	(setq tol 0.001 msa (/ pi 4.0) vlen 20.0)
	(setq drift (* pi (/ 10.0 180.0)))
	(if (> h msa)       (setq h msa))
	(if (< h (- 0 msa)) (setq h (- 0 msa)))
	(if (< d 0)         (setq d 0.0))
	(setq h (+ h drift))
	(setq turn (/ (* (tan h) d) vlen))
	(setq sx (cell t 0 0))
	(setq sy (cell t 1 0))
	(setq so (cell t 2 0))

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

; P D I controller

(def main (p-param d-param i-param )
	(setq turtle [0.0 1.0 0.0])
	(tprint turtle)
	(setq p1 (- 0 p-param) )
	(setq cte1 (cell turtle 1 0) sumcte 0.0)
	(for i '(1 100)
		(setq cte2 (cell turtle 1 0))
		(setq sumcte (+ sumcte cte2))
		(setq steer (- (- (* p1 cte2)  (* d-param (- cte2 cte1))) (* i-param sumcte)))
		(setq turtle (move turtle 1.0 steer))
		(setq cte1 cte2)
		(tprint turtle)
	)
	turtle
)

(main 0.2 3.0 0.004)


