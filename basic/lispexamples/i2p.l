!DO
(DEF WEIGHT (OP) (COND ((= OP 'DUMMY) -1              (= OP '=) 0     (= OP '+) 1      (= OP '*)  2      (PR 'NOP)  )))
(DEF OPC    (OP) (COND ((= OP 'DUMMY) (PR 'DUMMY-HIT) (= OP '=) 'SETQ (= OP '+) 'PLUS  (= OP '*) 'TIMES  (PR 'NOP)  )))

(DEF I2P (AE) 
  (WITH OPD OPS SCAN STUFF) 
  (SETQ SCAN 1 STUFF 1)
  (IF (ATOM AE) (RETURN AE) 
  (DO 
     (SETQ OPS (LIST 'DUMMY))
     (WHILE STUFF 
        (IF   (NULL AE) (RETURN 'UNEXP))
        (SETQ OPD (CONS (IF (ATOM (CAR AE)) (CAR AE) (I2P (CAR AE))) OPD)  AE (CDR AE) SCAN 1)
        (WHILE SCAN 
           (IF (AND (NULL AE) (=  (CAR OPS) 'DUMMY)) (RETURN (CAR OPD)))
           (IF  (OR (NULL AE) (<= (WEIGHT (CAR AE)) (WEIGHT (CAR OPS))))
             (SETQ OPD (CONS (LIST (OPC (CAR OPS)) (CAR (CDR OPD)) (CAR OPD)) (CDR (CDR OPD))) OPS (CDR OPS))
             (SETQ OPS (CONS (CAR AE) OPS) SCAN 0)
	   )
        )
        (SETQ AE (CDR AE))
     )
  )
))

(SETQ EQ '(C = A + B * C))
(PR EQ " -> " (I2P EQ))



