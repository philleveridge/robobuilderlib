!DO
(SETQ C '(0 1 2))
(SETQ D '( (0 1) (1 0) (-1 0) (0 -1)))
(SETQ E '( (1 1 2) (2 2 2) (1 1 1) (1 1 3)))

(FUNC FOO (X Y)
  (WITH C1 C2 L)
  (SETQ (C1 C2 L) X)
  (SETQ R)
  (FOREACH P D
     (SETQ Z (LIST (+ C1 (CAR P)) (+ C2 (CAR (CDR P))) (+ L 1)))
     (IF (NOT (NULL (TEST Z)))  (SETQ R (CONS Z R)))
  )
  (PR R))


(FUNC TEST (X)
   (WITH C1 C2 L)
   (SETQ (C1 AC L) X)
   (IF (OR (< C1 0) (< C2 0)) () X)
)

(FUNC CHOOSE (X)
  (WITH C1)
  (SETQ C1 (CAR X))
  (WHILE (NOT (NULL X))
    (SETQ X (CDR X))
    (IF (> (NTH 2 C1) (NTH 2 (CAR X))) (SETQ C1 (CAR X)) )
  )
  (PR C1)
)

(PR "Candidates : " (FOO C D))

(PR "Choose best "  (CHOOSE E))

'(EXIT)


