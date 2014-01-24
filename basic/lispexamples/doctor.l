!DO

(DEF DOCTOR ()
(SETQ LP 1)
(WHILE LP
  (SETQ INP (READ))
  (COND ( (= (CAR INP) 'BYE)                 (RETURN "See you")
        (MATCH '(I AM WORRIED ABOUT +L) INP) (PR "WHY ARE YOU WORRIED ABOUT " L "?")
        (MATCH '(+ MOTHER +) INP)            (PR "TELL ME MORE ABOUT YOUR FAMILY")
        (PR "TELL ME MORE"))
  )
))
 
(DEF MATCH (P D)
'(PR "P=" P " D=" D)
(COND ((AND (NULL P) (NULL D))                               1
       (OR  (NULL P) (NULL D))                               1
       (OR  (= (CAR P) '>) (= (CAR P) (CAR D)))              (MATCH (CDR P) (CDR D))
       (AND (= (LFT (CAR P) 1) '>) (MATCH (CDR P) (CDR D)))  (SET (MID (CAR P) 2) (CAR D))
       (= (CAR P) '+)                                        (COND ((MATCH (CDR P) (CDR D)) 1 (MATCH P (CDR D))))
       (= (LFT (CAR P) 1) '+) (COND ((MATCH (CDR P) (CDR D)) (SET (MID (CAR P) 2) (LIST (CAR D)))
                                     (MATCH P (CDR P))       (SET (MID (CAR P) 2) (CONS (CAR D) (MID (CAR P) 2)))))
      )
))

;initialise
(SETQ L 0)
 
;test
(PR "T1  E=1 "  (MATCH '(A B C)  '(A B C)))
(PR "T2  E=1 "  (MATCH '(A B +)  '(A B C)))
(PR "T3  E=1 "  (MATCH '(A B +)  '(A B C D)))
(PR "T4  E=0 "  (MATCH '(A B D)  '(A B C)))
(PR "T5  E=1 "  (MATCH '(A B +L) '(A B C))    " L=(C)   " L)
(SETQ L 0)
(PR "T6  E=1 "  (MATCH '(A B +L) '(A B F D))  " L=(F) " L)
(SETQ L 0)
(PR "T7  E=1 "  (MATCH '(A B >)  '(A B E )) )
(PR "T8  E=1 "  (MATCH '(A B >L) '(A B E ))   " L=" L)
(PR "T9  E=1 "  (MATCH '(+ B +)  '(A B C )) )
(PR "T10 E=1 "  (MATCH '(+L B +) '(A B C D)) " L=" L)
