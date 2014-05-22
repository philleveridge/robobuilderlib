!do
;kalman_m.l

(def filter ( X mP)
	(setq z [0])
   	(foreach el mP 
		(pr "el=" el)
		(setc z 0 0 el)
;measurement
		(setq y (- Z (* H x)))
		(setq s (+ (* H (* P (TRN H))) R))
		(setq k (* (* P (TRN H)) (INV s)))
		(setq x (+ X (* K Y)))
		(setq P (* (- I (* K H)) P))

;prediction
		(setq x (+ (* F X) u))
		(setq p (* (* F P) (TRN F)))

; output
		(pr "X = " X) 
		(pr "P = " P)
		(pr) (pr)
	)
)

(setq measurements [1 2 3])

;initial state
(setq x   [0 ; 0])

;initial uncertainty
(setq P   [1000  0; 0 1000])

;external motion
(setq u   [0 ; 0])

;next state
(setq F   [1  1; 0 1])

;measurement function
(setq H   [1 0])

;measurement uncertainty
(setq R   [1])

;identity
(setq I   [1 0 ; 0 1])

(filter x measurements)

(exit)
