!DO 

;state space
; X _ # _ _ _ 
; _ _ # _ _ G
; _ _ # _ _ _
; _ _ _ _ _ #
; _ _ _ _ _ _
;

(SETQ WORLD '( (6 5) ( (3 1) (3 2) (3 3)) ))
;

(func PW (X S G) 
   (SETQ W (CAAR X) H (CADAR X) O (CADR X)) 
   (FOR I '(1 H) 
        (FOR J '(1 W) (COND ( (MEMBER (LIST J I) O) (PR '# ;)
                              (= S (LIST J I))      (PR 'S ;)
                              (= G (LIST J I))      (PR 'G ;)
                              (PR '_ ;)))) 
   (PR)))


(func GPATH (A W) 
   (SETQ (X Y) A WX (CAAR W) WY (CADAR W) R '()) 
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
			(SETQ nn (CONS z nn)))) nn)

(func CHECK (x y) 
	(FOREACH z y (IF (= (CAR z) x) (RETURN 1))))


(func GSEARCH (init goal) 
   (WITH path ns cost pl top)
   (PR "starting from .. " init " going to " goal ) 

   (PW WORLD init goal)

   (setq FRONTIER (LIST (LIST init 0 '())) EXPLORED '())

   (WHILE (NOT (NULL FRONTIER)) 
       	(SETQ top (CHOICE-SP FRONTIER))
	(IF (NULL top) (RETURN 'FAIL))
      	(setq FRONTIER (REMOVE top FRONTIER))
  	(SETQ EXPLORED (CONS top EXPLORED))
	(SETQ (ns pl path) top)
       	(IF (SETQ action  (GPATH ns WORLD))
        (DO
		(SETQ path (CONS ns path))
		(IF (=  ns goal) (RETURN (REV path)) )
		(inc pl)
	       	(FOREACH ns action 
		    (iF (AND (NOT (CHECK ns EXPLORED)) (NULL (MEMBER ns (CADR WORLD))))
		         (setq FRONTIER (CONS (LIST ns pl path) FRONTIER)))
	       	)
        ))
   )
   'FAIL
)


;testing
;!> GPATH '(1 2)    
; = ((1 3) (1 1) (2 2) (0 2))
;!> GSEARCH '(1 1) '(4 5)
;starting from .. (1 1) going to (4 5)

(PR "Path = " (GSEARCH '(1 1) '(4 5)))

(PR "Path = " (GSEARCH '(1 1) '(5 1)))
'done





