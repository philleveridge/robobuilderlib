!do
; smooth by gradient decsent and

; Yi (x,y) += alpha*(xi-yi) + beta*(y(i+1) +  y(i-1) - 2*yi)

(SETQ WORLD '( (6 5) ( (3 1) (3 2) (3 3)) ))
;

(func PW (X S G) 
   (SETQ W (CAAR X) H (CADAR X) O (CADR X)) 
   (FOR I '(1 H) 
        (FOR J '(1 W) (COND   (MEMBER (LIST J I) O) (PR '# ;)
                              (= (LIST J I) S) (PR 'S ;)
                              (= (LIST J I) G) (PR 'G ;)
                              (PR '_ ;)))
   (PR)))


(func GPATH (A W) 
   (SETQ (X Y) A)
   (SETQ  WX (CAAR W) WY (CADAR W) R '()) 
   (IF (< Y WY) (SETQ R (CONS (LIST X (+ Y 1)) R)))
   (IF (< X WX) (SETQ R (CONS (LIST (+ X 1) Y) R)))
   (IF (> X 1)  (SETQ R (CONS (LIST (- X 1) Y) R)))
   (IF (> Y 1)  (SETQ R (CONS (LIST X (- Y 1)) R)))
   R
)

(func CHOICE-SP (f) 
        (WITH (MP 99) BS x) 
        (FOREACH x f 
            (IF (< (CADR x) MP) 
                 (SETQ MP (CADR x) BS x) 
            ))
       '(PR "Best SP=" MP BS )  BS)

(func REMOVE (a b) 
	(WITH nn z) 
	(FOREACH z b (IF (NOT (= (CAR z) (CAR a))) 
			(SETQ nn (CONS z nn))))
	nn)

(func CHECK (x y) 
	(FOREACH z y (IF (= (CAR z) x) (RETURN 1))))


(func GSEARCH (init goal) 
   (WITH path ns cost pl top)
   (PR "starting from .. " init " going to " goal ) 

   (PW WORLD init goal)

   (setq FRONTIER (LIST (LIST init 0 '())) EXPLORED '())

   (WHILE (NOT (NULL FRONTIER)) 
       	(SETQ top (CHOICE-SP FRONTIER))
	(IF (NULL top) (RETURN "Fail"))
      	(setq FRONTIER (REMOVE top FRONTIER))
  	(SETQ EXPLORED (CONS top EXPLORED))
	(SETQ (ns pl path) top)
       	(IF (SETQ action  (GPATH ns WORLD))
        (DO
		(SETQ path (CONS ns path))
		(IF (=  ns goal) (RETURN (REV path)) )
		(inc 'pl)
	       	(FOREACH ns action 
		    (iF (AND (NOT (CHECK ns EXPLORED)) (NULL (MEMBER ns (CADR WORLD))))
		         (setq FRONTIER (CONS (LIST ns pl path) FRONTIER)))
	       	)
        ))
   )
   'FAIL
)

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

(def ptom (p)
	(setq c 0 m (zero (len p) 2))
	(foreach e p 
		(setc m 0 c (car e))
		(setc m 1 c (cadr e))
		(inc 'c)
	)
	m
)

;testing

;(PR "Path = " (setq p (GSEARCH '(1 1) '(4 5))))

(PR "Path = " (setq p (GSEARCH '(1 1) '(5 1))))

(pr (smooth (ptom p) 0.5 0.1))


'done

