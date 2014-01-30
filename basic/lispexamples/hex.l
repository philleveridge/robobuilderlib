!DO

(func PB (X) 
    (PR (NTH 0 X) ; (NTH 1 X) ; (NTH 2 X) "  ... 0 1 2 ")
    (PR (NTH 3 X) ; (NTH 4 X) ; (NTH 5 X) "  ... 3 4 5 ")
    (PR (NTH 6 X) ; (NTH 7 X) ; (NTH 8 X) "  ... 6 7 8 ")
)

(func CHECKM (P X Y) 
      (WITH (F (/ X 10)) (T (MOD X 10))) 

      (IF (OR (= P (NTH T Y)) (<> P (NTH F Y))) (RETURN 0))
      (IF (= (NTH T Y) '_) 
             (IF (OR (AND (= P 'C) (= T (+ F 3) ))  (AND (= P 'H) (= T (- F 3) ))) (RETURN 1))
	     (COND ( 
                  (AND (= P 'C)  (OR (= T (+ F 4)) (= T (+ F 2))) (= (- (/ T 3) (/ F 3)) 1)) (RETURN 1)
                  (AND (= P 'H)  (OR (= T (- F 4)) (= T (- F 2))) (= (- (/ F 3) (/ T 3)) 1)) (RETURN 1))
	     )
      )
      0
)

(func CS (PUTC 27) (PR "[2J"))

(func MOVE (P M B) 
         (WITH (L '()) T F) 
	 (SETQ (F T) M)
         (FOR I '(8 0) 
              (SETQ L (CONS (COND ( (= I F) '_ (= I T) P (NTH I B)) ) L )))  
         L)

(func CMPB (A B) (OR (= A B) ) 
             (= A (LIST (NTH 2 B) (NTH 1 B) (CAR B) 
                        (NTH 5 B) (NTH 4 B) (NTH 3 B) 
                        (NTH 8 B) (NTH 7 B) (NTH 6 B))))


(func HEX() (WITH STORE )
(WHILE 1
  (setq BOARD '(C C C _ _ _ H H H) )

  (WHILE 1
	  (PB BOARD)
	  (Setq C 0)
	  (FOR M '(10 87) (SETQ C ( + C (CHECKM 'H M BOARD))))
	  (PR "valid moves for Human = " C)
          (IF (= C 0) (BREAK (PR "You Loose")))
	  (setq loop 1)
          (WHILE loop
		  (PR "Input your move (or Q): " ;)
		  (SETQ M (READ))
		  (IF (= (CAR M) 'Q) (RETURN 'Quitter))
		  (PR "From " (CAR M) " To " (CADR M))
		  (IF (NOT (CHECKM 'H (+ (* (CAR M) 10) (CADR M)) BOARD)) 
		             (PR "***** Bad move") 	  
		             (SETQ BOARD (MOVE 'H M BOARD) loop 0))
          )

	  (PR "test " (SETQ m (FOREACH b STORE (IF (CMPB b BOARD) (BREAK 1)) )))

	  (SETQ STORE (CONS BOARD STORE)) '(PR STORE)
   	   
	  (PR "My move : " )
	  (Setq C 0 MM 0)
	  (FOR M '(0 78) (IF (CHECKM 'C M BOARD) (DO (INC C) (SETQ MM M))))
	  (PR "valid moves for Computer = " C " .. " MM)
          (IF (= C 0) (PR (BREAK "You Win")))
	  (SETQ BOARD (MOVE 'C (LIST (/ MM 10) (MOD MM 10)) BOARD))
;
; need to add logic here to pick best move
;
  )
  (PR "Again ?")
  (IF (= (READ) '(N)) (RETURN 'DONE))
))

(PR "tst = " (CMPB '(C C C _ _ _ H H H) '(C C C _ _ _ H H H) ))
(PR "tst x " (CMPB '(_ C C _ _ _ H _ _) '(C C _ _ _ _ _ _ H) ))

(HEX)



