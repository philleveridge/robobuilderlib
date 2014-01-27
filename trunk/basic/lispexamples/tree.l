!DO 

;state space
(setq STATE '( 
	(TIMIS     (LOGOJ 111) (ARAD 118))
	(ZERIN     (ARAD 75)   (ORADCA 71))
	(LUGOJ     (MEHAD 70)  (TIMIS 111))
	(FAGARAS   (SIBIU 99)  (BUCHAREST 211))
	(ARAD      (ZERIN 75)  (SIBIU 140)   (TIMIS 118))
	(RIMNI     (SIBIU 80)  (PITESTI 97)  (PLACE 145))
	(PITESTI   (RIMI 97)   (PLACE 138)   (BUCHAREST 101))
	(BUCHAREST (URZI 85)   (PITESTI 101) (FAGARAS 211))
	(SIBIU     (ARAD 140)  (ORADCA 151)  (FAGARAS 99) (RIMNI 80))
))

; Direct distances to bucharest from Arad
(setq H '(
	(ARAD      366)
	(TIMIS     329)
	(LOGOJ     244)
	(ZERIN     374)
	(SIBIU     253)
	(RIMNI     193)
	(FAGARAS   176)
	(PITESTI   100)
	(BUCHAREST 0)))

(func GPATH (x) 
	(FOREACH y STATE 
		(IF (= (CAR y) x) (BREAK (PR x " .. " (CDR y)))) ))

(func CHOICE-SP (f) 
        (WITH (MP 99) BS x) 
        (FOREACH x f 
            (IF (< (CAR (CDR x)) MP) 
                 (SETQ MP (CAR (CDR x)) BS x) 
            ))
       '(PR "Best SP=" MP BS )  BS)

(func HVAL (x) (FOREACH y H (IF (= (CAR y) x) (RETURN (CADR y)))) '1000)

(func CHOICE-AS (f) 
        (WITH (LC 1000) BS x C ) 
        (FOREACH x f 
	    (SETQ C (+ (NTH 2 x) (HVAL (CAR x))))
            (IF (< C LC) 
                 (SETQ LC C BS x) 
            ))
       '(PR "Best A*=" LC BS) BS)

(func CHOICE-LC (f) 
        (WITH (LC 500) BS x) 
        (FOREACH x f 
            (IF (< (NTH 2 x) LC) 
                 (SETQ LC (NTH 2 x) BS x) 
            ))
       '(PR "Best LC=" LC BS) BS)

(func REMOVE (a b) 
	(WITH nn z) 
	(FOREACH z b (IF (NOT (= (CAR z) (CAR a))) 
			(SETQ nn (CONS z nn)))) nn)

(func CHECK (x y) 
	(FOREACH z y (IF (= (CAR z) x) (RETURN 1))))


(func GSEARCH (init goal meth) 
   (WITH path ns cost pl top)
   (PR "starting from .. " init " going to " goal )  (PR meth)
   (setq FRONTIER (LIST (LIST init 0 0)) EXPLORED '())

   (WHILE (NOT (NULL FRONTIER)) 

       	(SETQ top (COND ( (= meth 'LC) (CHOICE-LC FRONTIER) 
                          (= meth 'SP) (CHOICE-SP FRONTIER)
                          (= meth 'AS) (CHOICE-AS FRONTIER))))

	(IF (NULL top) (RETURN 'FAIL))

      	(setq FRONTIER (REMOVE top FRONTIER))
  	(SETQ EXPLORED (CONS top EXPLORED))
	(SETQ (ns pl cost path) top)

       	(IF (NOT (NULL (SETQ action  (GPATH ns)) ))
        (DO
		(SETQ path (CONS ns path))
		(IF (=  ns goal) (DO (PR "Cost=" cost) (RETURN (REV path)) ))
		(inc pl)
	       	(FOREACH a action 
		    (SETQ (ns cs) a)
		    (iF (NOT (CHECK ns EXPLORED))
		    (setq FRONTIER (CONS (LIST ns pl (+ cs cost) path) FRONTIER)))
	       	)
        ))
   )
   'FAIL
)

;testing
'(SETQ   B1 (CHOICE-LC '( (A 2 5) (B 5 7 ) (C 1 23))))
'(PR "test " (REMOVE B1 '( (A 2) (B 1) (C 3))))
'(PR (GSEARCH 'ARAD 'ZERIN'))

;init arad - goal bucharest
(PR (GSEARCH 'ARAD 'BUCHAREST' 'LC))
(PR (GSEARCH 'ARAD 'BUCHAREST' 'SP))
(PR (GSEARCH 'ARAD 'BUCHAREST' 'AS))
'done





