!do
;Planet		A (input)  	P (output) 
(setq data '(
(Venus 		0.72 		0.61	)
(Earth   	1.0 		1.0	)
(Mars    	1.52 		1.87	)
(Jupiter 	5.2 		11.9	)
(Saturn  	9.53 		29.4	)
(Uranus  	19.1 		83.5	)))

;A set of fitness cases, with planetary data taken from Urey (1952). A is
;Earths semi-major axis of orbit and P is in units of earth-years.


(setq foo '( 
	(( A A A )  2 +  )
	(( A A B )  2 -  ) 
	(( A A C )  2 *  ) 
	(( A B A )  2 /  ) 
	(( A B B )  2 +  ) 
	(( A B C )  2 -  ) 
	(( A C A )  2 *  ) 
	(( A C B )  2 +  ) 
	(( A C C )  2 -  ) 

	(( B A A )  1 SIN   ) 
	(( B A B )  1 SQRT  ) 
	(( B A C )  1 COS   ) 
	(( B B A )  1 LOG   ) 
	(( B B B )  1 SQRT  ) 

	(( B B C )  0 X  ) 
	(( B C A )  0 0  ) 
	(( B C B )  0 X  ) 
	(( B C C )  0 X  ) 

	(( C A A )  0 0  ) 
	(( C A B )  0 1  )
	(( C A C )  0 2  )
	(( C B A )  0 3  )
	(( C B B )  0 X  )
	(( C B C )  0 5  )
	(( C C A )  0 X  )
	(( C C B )  0 7  )
	(( C C C )  0 X  )
))

(setq nil '())

(def cop  (N)  (INT (* N (RND))))

(def base (N)  (nth (INT (* 3 (RND))) '(A B C)))


(def newlife () 
	(for x (list 0 (+ 20 (cop 15))) 
		(setq lst (cons (base) lst))))


(def  triple  (a) (list (nth 0 a) (nth 1 a) (nth 2 a) ))
(def  trest   (a) (cdr(cdr(cdr a))))

(def  express (t) 
;	(pr "[" t "]")
	(if (null t) (return ))
	(foreach m foo 
		(if (= (sum (ham (car m) t )) 3) 
			(return (list (cadr m) (nth 2 m)))
		)
	)
	(list 0 0)
)

(def trans (t)
	(with h1 h2)
	(if (null t) (return nil))
	(setq h1 (express (triple t)))
	(cond 
	(= (car h1) 2) 
		(return (list (cadr h1) (trans (setq t  (trest t))) (trans (setq t  (trest t))  )))
	(= (car h1) 1) 
		(return (list (cadr h1) (trans (setq t  (trest t)))   ))

	(= (car h1) 0) 
		(return  (cadr h1) )
	)
)


(def mate (X Y C1 C2) 
   (with T )
;  (pr "x=" x " y=" y " c1=" c1 " c2=" c2)
   (IF (and (NULL X) (NULL Y)) (return nil))
   (IF (NULL X) (return (car y)))
   (IF (NULL Y) (return (car x)))
   (SETQ T (IF (and (<= C1 0) (>= C2 0))  (CAR y) (CAR x)))
   (CONS T (mate (CDR X) (CDR Y) (- C1 1) (- C2 1)))
)

(def testfit (fn tdata)
	(setq fit 0)
;	(pr "fn=" fn)
	(foreach tt tdata 
;		(pr (car tt))
		(setq x (cadr tt))
		(setq out (nth 2 tt))
		(setq s (eval fn))
		(if (and (> (/ out s) 0.9)  (< (/ out s) 1.1)) (inc 'fit))
	)
	fit

)

(setq PM 0.0 PI 0.0 PD 0.5)

(def mutate (a)
	(cond   (null a) nil
		(atom a) 
			(cond (< (RND) PM) (base)
			a)
		(< (rnd) PI) (cons (base)            (mutate a))  
		(< (rnd) PD) (mutate (cdr a))
		(cons (mutate (car a)) (mutate (cdr a)))
	)
)

(def breed (a b)
	(setq lst nil)
	(for i '(1 5) 
		(setq ln (len a))
		(setq c1 (COP ln))
		(setq c2 (+ c1 (COP (- ln c1))))
		(PR (setq sq (MATE a b c1 c2)))
		(setq f (trans sq))
		(setq lst (cons (list (testfit f DATA) sq) lst))
	)
	(pr "lst=" (rev (sort lst)))
)

(def main ()
	(setq PM 0.1 PI 0.1 PD 0.1)
	(setq pool (list (newlife) (newlife) (newlife) (newlife)))

	(for g '(1 3)
		(setq p1 (car pool) p2 (cadr pool))
		(setq pool (cdr (cdr pool)))
		(pr "gen " g " nom1=" (trans p1) " nom2=" (trans p2))
		
		(setq r (breed p1 p2))
		(setq r1 (pr "r1= " (car r)))
		(setq r2 (pr "r2= " (cadr r)))

		(nconc pool (cadr r1) )
		(nconc pool (cadr r2) )
	)
)

(main)










