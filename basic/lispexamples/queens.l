!DO 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; QUEEN 8
; Board=((8 4) (7 2) (6 7) (5 3) (4 6) (3 8) (2 5) (1 1))
; Q _ _ _ _ _ _ _ 
; _ _ _ _ Q _ _ _ 
; _ _ _ _ _ _ _ Q 
; _ _ _ _ _ Q _ _ 
; _ _ Q _ _ _ _ _ 
; _ _ _ _ _ _ Q _ 
; _ Q _ _ _ _ _ _ 
; _ _ _ Q _ _ _ _ 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


(FUNC THREAT (I J A B)  (OR (= I A) (= J B) (= (- I J) (- A B)) (= (+ I J) (+ A B)) ))

(FUNC CONFLICT (N M BOARD) (IF  (NOT (NULL BOARD)) 
    (OR (THREAT N M (CAAR BOARD) (CADAR BOARD))   (CONFLICT N M (CDR BOARD)))))


(DEF PBOARD (SIZE BOARD) 
  (FOR 'N '(1 SIZE) 
    (FOR 'M '(1 SIZE) 
       (IF (NULL (MEMBER (LIST N M) BOARD))  (PR '_ ;) (PR 'Q ;))
    )
  (PR)))


(DEF QUEEN (SIZE) 
  (WITH N M BOARD F) 
  (SETQ N 1)
  (WHILE  (<= N SIZE)
     (SETQ M 1 F 0)
     (WHILE (<= M SIZE) 
        '(PRINT N M ;)
        (IF (NOT (CONFLICT N M BOARD)) 
              (DO (SETQ BOARD (CONS (LIST N M) BOARD) F 1) '(PR 'OK BOARD))
        )

	(WHILE  (AND (NOT (NULL BOARD)) (= F 0) (>= M SIZE)) 
            (SETQ M (CADAR BOARD) N (CAAR BOARD) BOARD (CDR BOARD))
        )

        (INC M)
     )
     (INC N)
  )
  (PR "Board=" BOARD) (PBOARD SIZE BOARD))


(QUEEN 8)


