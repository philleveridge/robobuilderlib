!DO
(DEF WEIGHT (OP) (COND ((= OP 'DUMMY) -1              (= OP '=) 0     (= OP '+) 1      (= OP '*)  2      (PR 'NOP)  )))
(DEF OPC    (OP) (COND ((= OP 'DUMMY) (PR 'DUMMY-HIT) (= OP '=) 'SETQ (= OP '+) 'PLUS  (= OP '*) 'TIMES  (PR 'NOP)  )))

;LIST based
(DEF I2P (AE) 
  (WITH OPD  (OPS (LIST 'DUMMY)) (SCAN 1) (STUFF 1) )
  (IF (ATOM AE) (RETURN AE) 
  (DO 
     (WHILE STUFF 
        (IF   (NULL AE) (RETURN 'UNEXP))
        (SETQ OPD (CONS (IF (ATOM (CAR AE)) (CAR AE) (I2P (CAR AE))) OPD)  AE (CDR AE) SCAN 1)
        (WHILE SCAN 
           '(PR " ae = " AE " OPD = " OPD " OPS=" OPS)
           (IF (AND (NULL AE) (=  (CAR OPS) 'DUMMY)) 
             (RETURN (CAR OPD)))
           (IF  (OR (NULL AE) (<= (WEIGHT (CAR AE)) (WEIGHT (CAR OPS))))
             (SETQ OPD (CONS (LIST (OPC (CAR OPS)) (CAR (CDR OPD)) (CAR OPD)) (CDR (CDR OPD))) OPS (CDR OPS))
             (SETQ OPS (CONS (CAR AE) OPS) SCAN 0)
	   )
        )
        (SETQ AE (CDR AE))
     )
  )
))

'(DO (PR "ae  = " AE)(PR "OPD = ";) (PST OPD)(PR "OPS = ";) (PST OPS))

(DEF PST (X) (FOREACH Y X (PR Y ".." ;)) (PR ""))

;stack based version
(DEF I2P2 (AE) 
  (WITH (OPD (STACK 5)) (OPS (STACK 5))) 
  (IF (ATOM AE) (RETURN AE) 
  (DO 
     (WHILE 1
        (IF   (NULL AE) (RETURN 'UNEXP))
        (PUSH OPD (IF (ATOM (CAR AE)) (CAR AE) (I2P (CAR AE))))  
        (SETQ AE (CDR AE))
        (WHILE 1    
           (IF (AND (NULL AE) (=  (LEN OPS) 0)) 
	      (RETURN (POP OPD)))
           (IF (OR (NULL AE)  (<= (WEIGHT (CAR AE)) (WEIGHT (PEEK OPS 0))))
              (DO (SETQ B (POP OPD) A (POP OPD)) (PUSH OPD (LIST (OPC (POP OPS)) A B)))
              (DO (PUSH OPS (CAR AE)) (BREAK))
	   )
        )
        (SETQ AE (CDR AE))
     )
  )
))

(SETQ EQ '(C = A + B * C))
'(PR EQ " -> " (I2P EQ))

(PR EQ " -> " (I2P2 EQ))


