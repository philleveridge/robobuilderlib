!DO
(FUNC DEFPROP (L)
(DO 
(FOREACH X L (SET 'E (CONS(LIST X 0) E)))
(DICT E)
))

(FUNC PUTPROP (I V P) (SETK  (LIST I P V)))
(FUNC REMPROP (I P)   (SETK  (LIST I P )))
(FUNC GETPROP (I P )  (GETK  (LIST I P )))

(SET 'B1 (DEFPROP '(POSITION COLOR SIZE)))
(SET 'B2 (DEFPROP '(POSITION COLOR SIZE)))

(PUTPROP 'B1 'RED 'COLOR)
(PUTPROP 'B1 2.0 'SIZE)
(PUTPROP 'B2 3.0 'SIZE)

(PR B1 B2)

(DEF DOCTOR () 
(SETQ LP 1)
(WHILE LP (DO
(SETQ INP (READ))
(COND ( (= (CAR INP) 'BYE)                   (DO (SETQ LP 0) (PR "See you"))
        (MATCH '(I AM WORRIED ABOUT +L) INP) (PR "WHY ARE YOU WORRIED ABOUT " L "?")
        (MATCH '(+ MOTHER +) INP)            (PR "TELL ME MORE ABOUT YOUR FAMILY")
        (PR "TELL ME MORE"))
)))

(DEF MATCH (P D)
(COND ((AND (NULL P) (NULL D))                               1
       (OR  (NULL P) (NULL D))                               0
       (OR  (= (CAR P) '>) (= (CAR P) (CAR D)))              (MATCH (CDR P) (CDR D))
       (AND (= (LFT (CAR P)) ">") (MATCH (CDR P) (CDR D)))   (SET (STOA (MID 2 (CAR P))) (CAR D))
       (= (CAR P) '+)                                        (IF (MATCH (CDR P) (CDR D)) 1 (MATCH P (CDR D)))
       (= (LFT (CAR P) 1) '+) (COND ((MATCH (CDR P) (CDR D)) (SET (STOA (LFT (CAR P) 1)) (LIST (CAR D)))
                                     (MATCH P (CDR P))       (SET (STOA (LFT (CAR P) 1)) (CONS (CAR D) (MID (CAR P) 2)))))
      )
))

