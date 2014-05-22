!do

(def MODULO (X Y) 
	(IF (>= X 0) (MOD X Y) (+ Y (MOD X Y))))

(def Move (Xm n pe po pu)
	(SETQ sz (COL Xm) R (ZERO 1 sz))
	(FOR I (list 0 (- sz 1))
		(SETC R I 0 ( PLUS (* pe (CELL Xm (MODULO (- I n)  sz) 0))
		   		   (* po (CELL Xm (MODULO (- I (+ n 1)) sz) 0))
		    		   (* pu (CELL Xm (MODULO (- I (- n 1)) sz) 0))
		))		
	)
	R
)

(def  Sense (z ph pm l prb)  
	(setq i 0 t 0)
	(foreach el l 
		(setq t (cell prb i 0)) 
		(if (= el z) 
			(setc prb i 0 (* ph t )) 
			(setc prb i 0 (* pm t )) )
		(inc i)
	)
	prb
)

(setq 	motions '( 1 1 )
 	meas    '( Red Green )
 	M        [ 0.2 0.2 0.2 0.2 0.2 ]
 	D       '( Red Red Green Green Red))

(foreach me meas  
	(pr "measure - " me ; )
	(SETQ m (Sense me 0.6 0.2 D m))
	(SETQ m (/ m (SUM m)))

	(pr " Move - "   (car motions))
	(setq m (Move M (car motions) 0.8 0.1 0.1))
	(setq motions (cdr motions))
	(PR m)
)

;output : 0.084211 0.140000 0.185263 0.388421 0.202105 
