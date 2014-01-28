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

(func MOVE (P M B) 
         (WITH (L '()) T F) 
	 (SETQ (F T) M)
         (FOR I '(8 0) 
              (SETQ L (CONS (COND ( (= I F) '_ (= I T) P (NTH I B)) ) L )))  
         L)

(func HEX() 
  (setq BOARD '(C C C _ _ _ H H H))
  (WHILE 1
	  (PB BOARD)
	  (Setq C 0)
	  (FOR M '(10 87) (SETQ C ( + C (CHECKM 'H M BOARD))))
	  (PR "valid moves for Human = " C)
          (IF (= C 0) (BREAK "You Loose"))
	  (setq loop 1)
          (WHILE loop
		  (PR "Input your move : " ;)
		  (SETQ M (READ))
		  (IF (= (CAR M) 'Q) (RETURN 'Quitter))
		  (PR "From " (CAR M) " To " (CADR M))
		  (IF (NOT (CHECKM 'H (+ (* (CAR M) 10) (CADR M)) BOARD)) 
		             (PR "***** Bad move") 	  
		             (SETQ BOARD (MOVE 'H M BOARD) loop 0))
          )
	  (PR "My move : " )
	  (Setq C 0)
	  (FOR M '(0 78) (SETQ C ( + C (CHECKM 'C M BOARD))))
	  (PR "valid moves for Computer = " C)
          (IF (= C 0) (BREAK "You Win"))
;
; need to add logic here to pick best move
;
  )
)

(HEX)



