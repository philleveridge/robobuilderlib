!do
; smooth by gradient decsent and

; Yi (x,y) += alpha*(xi-yi) + beta*(y(i+1) +  y(i-1) - 2*yi)

(setq mp [ 0 0 ; 0 1 ; 0 2 ; 1 2 ; 2 2 ; 3 2 ; 4 2 ; 4 3 ; 4 4])

(def smooth (xi alpha beta)
	(setq tol 0.001 ch 1.0 yi xi nr (row xi))
	(while (> ch tol)
		(setq ch 0.0)
		(for i (list 1 (- nr 2))
			(for j '(0 1)
				(setq x  (cell yi j i))
				(setq ax (* alpha (- (cell xi j i) x)))
				(setq bx (* beta  (plus (cell yi j (+ i 1)) 
							(cell yi j (- i 1))
							(* -2 x))))

				(setc yi j i (plus x ax bx))

				(setq ch (+ ch (abs (+ ax bx))))	
			)
;			(pr (cell xi 0 i) "," (cell xi 1 i) " -- " ;)
;			(pr (cell yi 0 i) "," (cell yi 1 i)  "  " ch)	
		)		
	)
	yi
)

(pr (smooth mp 0.5 0.1))



