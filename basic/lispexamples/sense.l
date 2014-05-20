!do

(SETQ M [ 0   0   1   0   0  ])

(def MODULO (X Y) 
	(IF (>= X 0) (MOD X Y) (+ Y (MOD X Y))))

(def move (Xm n)
	(SETQ sz (COL Xm) R (ZERO 1 sz))

        (FOR I (list 0 (- sz 1)) 
           (SETC R I 0 (CELL Xm (MODULO (- I n) sz) 0))) 
	r
)

(pr m)
(pr (setq m (MOVE m  2)))
(pr (setq m (MOVE m -1)))


(SETQ P '( 0.2 0.2 0.2 0.2 0.2))
(setq D '( Red Red Green Green Red))

(def  sense (z ph pm l prb)  
     (pr l "," prb)
     (cond (null l) '() 
           (cons (if ( = (car l) z) (* (car prb) ph) (* (car prb) pm)) (sense z ph pm (cdr l) (cdr prb)))
     ))

(SETQ SN (SENSE "Red" 0.6 0.2 D P))
(SETQ SN (/ SN (SUM SN)))

(PR "SN = " SN)
