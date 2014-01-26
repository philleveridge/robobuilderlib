!DO

(SETQ C '(A T C G T T A C G))

(FUNC DNA ( X  ) (COND ((= X 'A) 'T  (= X 'G) 'C (= X 'C) 'G (= X 'T) 'A)))

(PR C " --> " (MAPCAR DNA C))

(FUNC TRIP (C) (LIST (CAR C) (CAR (CDR C)) (CAR (CDR (CDR C)))))

(TRIP C)

(FUNC MATE (X Y C1 C2) 
   (WITH T )
   (IF (OR (NULL X) (NULL Y)) (RETURN '()))
   (SETQ T (IF (> C1 0)(CAR X) (CAR Y)))
   (CONS T (MATE (CDR X) (CDR Y) (- C1 1) C2))
)


(FUNC COP (N) (PR "random = " (INT (* N (RND)))))

(PR (MATE '(A A A A A) '(B B B B B) (COP 5) 3))
(PR (MATE '(A A A A A) '(B B B B B) (COP 5) 3))
(PR (MATE '(A A A A A) '(B B B B B) (COP 5) 3))




