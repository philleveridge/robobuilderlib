!do
;

(setq NIL    '())
(def  varp   (x)   (and (atom x) (=  (lft x 1) '?)))
(def  constp (x)   (and (atom x) (<> (lft x 1) '?)))
(def  comp   (x y) (cond (null x) y (null y) x (list x y)))

(def  subst (x y z) 
	(cond 	(null z) 	NIL
		(or (null x) (null y)) z			 
		(atom z) 	(if (= z x) y z) 
		(cons (subst x y (car z)) (subst x y (cdr z))))) 

(def testp ()  (pr "Stop  ?" ; ) (setq inp (read)) (= "Y" (car inp)))

(def unify (e1 e2) 
	(with he1 he2 subs1 te1 te2 subs2)
;	(pr "Unify " e1 " <-> " e2)
	(cond 
		(and (null e1) (null e2))   	NIL
		(and (constp e1) (constp e2))   (if (= e1 e2)      NIL   'FAIL)
		(varp e1)                       (if (member e1 e2) 'FAIL (list e1 e2))
		(varp e2)                       (if (member e2 e1) 'FAIL (list e2 e1))
		(or (null e1) (null e2))        'FAIL
;otherwise:
		(do 
		  (setq he1   (car e1))
		  (setq he2   (car e2))
		  (setq subs1 (unify he1 he2))
		  (if (= subs1 'FAIL) (return 'FAIL))

		  (setq te1   (subst (car subs1) (cadr subs1) (cdr e1)))
		  (setq te2   (subst (car subs1) (cadr subs1) (cdr e2)))	 
		  (setq subs2 (unify te1 te2))
		  (if (= subs2 'FAIL) (return 'FAIL))
		  (comp subs1 subs2)
		)
	)
)

(setq DB '((LIKES A B) (LIKES A C ) (LIKES B C)) facts db)

; 
; Simple Prolog!
; prolog:  ?> -? LIKES A ?X
; ----- (?X B)
; Stop  ? ?> Y
; prolog:  ?> -? LIKES A ?X
; ----- (?X B)
; Stop  ? ?> N
; ----- (?X C)
; Stop  ? ?> N
; prolog:  ?> 
; 
(def prolog ()
	(pr "Simple Prolog!")
	(setq facts db)
	(while 1 
		(pr "prolog: " ; ) 
		(setq l (read) h (car l))  
		(cond
			(= h "QUIT")  (return 'DONE)
			(= h "FACTS") (pr "Fact DB:\n" facts)
			(= h "CLEAR") (setq facts db)
			(= h "-?")    (foreach f facts 
						(setq r (unify (cdr l) f))
						(if (not (= r 'FAIL)) 
							(do (pr "----- " r) 
							(if (testp) (break))))) 
			(if (constp h) 
				(do (pr "fact -> " l) (nconc facts l))
				(pr "? err"))
		)
	)
)

(pr (prolog))




