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
;Earth s semi-major axis of orbit and P is in units of earth-years.


(setq foo '( 
	((A A B) 2 +    ) 
	((B A A) 2 -    ) 
	((C A A) 2 /    ) 
	((A B A) 2 *    )  
	((C C A) 1 SQRT ) 
	((C B A) 1 SIN  ) 
	((C C B) 1 COS  ) 
	((C C C) 0 X    ) 
	((D A A) 0 1    ) 
	((D A B) 0 3    ) 
	((C A C) 0 5    ) 
	((D D A) 0 7    ) 
	((D D B) 0 8    ) 
))


(def  triple  (a) (list (nth 0 a) (nth 1 a) (nth 2 a) ))
(def  trest   (a) (cdr(cdr(cdr a))))

(def  express (t) 
;	(pr "[" t "]")
	(if (null t) (return ))
	(foreach m foo 
		(if (= (sum ( = (car m) t )) 3) 
			(return (list (cadr m) (nth 2 m)))
		)
	)
	(list 0 0)
)

(def trans (t)
	(with h1 h2)
	(if (null t) (return '()))
	(setq h1 (express (triple t)))
	(cond 
	(= (car h1) 2) 
		(return (list (cadr h1) (trans (setq t  (trest t))) (trans (setq t  (trest t))  )))
	(= (car h1) 1) 
		(return (list (cadr h1) (trans (setq t  (trest t)))   ))

	(= (car h1) 0) 
		(return  (cadr h1) )
	)

	(pr "h1 = " h1)
)

(pr "t1 = " (trans '(C C A D A A)))
(pr "t2 = " (trans '(A B A C C C B B B)))
(pr "t3 = " (trans '(C C A A B A C C C D A B)))
(setq x 9) 
(pr (eval (trans '(C C A A B A C C C D A B))))

;(setq nome '(C C A  C C C  C C C))
;(pr "t3 = " (trans nome))




