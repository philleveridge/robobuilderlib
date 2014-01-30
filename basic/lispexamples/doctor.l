!DO

(DEF MATCH (P D)
'(PR "P=" P " D=" D)
(COND ((AND (NULL P) (NULL D))                               1
       (OR  (NULL P) (NULL D))                               0
       (OR  (= (CAR P) '>) (= (CAR P) (CAR D)))              (MATCH (CDR P) (CDR D))
       (AND (= (LFT (CAR P) 1) '>) (MATCH (CDR P) (CDR D)))  (SET (MID (CAR P) 2) (CAR D))
       (= (CAR P) '+)                                        (COND ((MATCH (CDR P) (CDR D)) 1 (MATCH P (CDR D))))
       (= (LFT (CAR P) 1) '+) (COND ((MATCH (CDR P) (CDR D)) (SET (MID (CAR P) 2) (LIST (CAR D)))
                                     (MATCH P (CDR P))       (SET (MID (CAR P) 2) (CONS (CAR D) (MID (CAR P) 2)))))
      )
))

;tests
'(
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
)

(SETQ DATA  '(
   ((I AM WORRIED ABOUT +L )  	("What are you worried about " L "?") )

   ((+ MOTHER +)              	("Tell me more about your family"))

   ((+ COMPUTER +)            	(("Are you afarid of computers?")
                              	("Do computers worry you ?")
				("Why do you mention computers ?")
				("What do you think machines have to do with your problem ?")))

   ( (+ YES +)                  (("You seem to be quite positive.")
				("You are sure.")
				("I see.")
				("I understand")))

   ( (WHAT +)                   (("Why do you ask ?")
				("Does that question interest you ?")
				("What is it you really wanted to know ?")
				("Are such questions much on your mind ?")
				("What answer would please you most ?")
				("What do you think ?")
				("What comes to mind when you ask that ?")
				("Have you asked such questions before ?")
				("Have you asked anyone else ?")))

   ( (BECAUSE + )               (("Is that the real reason ?")
				("Don't any other reasons come to mind ?")
				("Does that reason seem to explain anything else ?")
				("What other reasons might there be ?")))

   ( (WHY DONT YOU +L)          (("Do you believe I don't " L " ?")
				("Perhaps I will" L " in good time.")
				("Should you " L " yourself ?")
				("You want me to " L " ?")))
))

(func GETRES (data inp)
   (FOREACH x data 
        (IF (MATCH (CAR x) inp)  
            (IF  (> (LEN (CADR x)) 1) 
                 (RETURN (CONS 'PR (NTH (INT (* (LEN (CADR x)) (RND))) (CADR x) )))  
                 (RETURN (CONS 'PR (CADR x)))
            ) 
        )
   ) 0 )


;initialise
(SETQ L 0)
 

(DEF DOCTOR ()
(SETQ LP 1)
(WHILE LP
  (SETQ INP (READ))
  (COND ( (= (CAR INP) 'BYE)             (RETURN "See you")
          (SETQ ANS (GETRES DATA INP))   (EVAL ANS)
          (PR "TELL ME MORE"))
  )
))
 
'( GETRES DATA '(A YES B) )

(DOCTOR)



