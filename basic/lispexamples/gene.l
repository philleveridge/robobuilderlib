!DO

; 1. Phenylalanine (Phe): UUU, UUC
; 2. Leucine (Leu): UUA, UUG
; 3. Isoleucine (Ile): AUU, AUC, AUA
; 4. Methionine (Met): AUG
; 5. Valine (Val): GUU, GUC, GUA, GUG
; 6. Serine (Ser): UCU, UCC, UCA, UCG, AGU, ACG
; 7. Proline (Pro): CCU, CCC, CCA, CCG
; 8. Threonine (Thr): ACU, ACC, ACA, ACG
; 9. Alanine (Ala): GCU, GCC, GCA, GCG
;10. Tyrosine (Tyr): UAU, UAC
;11. Histidine (His): CAU, CAC
;12. Glutamine (Gln): CAA, CAG
;13. Asparagine (Asn): AAU, AAC
;14. Lysine (Lys): AAA, AAG
;15. Aspartic acid (Asp): GAU, GAC
;16. Glutamic acid (Glu): GAA, GAG
;17. Cysteine (Cys): UGU, UGC
;18. Tryptophan (Trp): UGG
;19. Arginine (Arg): CGU, CGC, CGA, CGG, AGA, AGG
;20. Glycine (Gly): GGU, GGC, GGA, GGG
;In addition, the following triplet codes are special:
;STOP: UAA, UAG, UGA
;START: AUG (same code as the Methionine amino acid)

(SETQ C '(A T C G T T A C G))

(FUNC DNA ( X  ) (COND (= X 'A) 'U  (= X 'G) 'C (= X 'C) 'G (= X 'T) 'A))

(PR C " --> " (MAPCAR DNA C))

(FUNC TRIP (C) (LIST (CAR C) (NTH 1 C) (NTH 2 C)))
(FUNC NEXT (X) (CDR (CDR (CDR X))))

(SETQ START '( (U A A) (U A G) (U G A)))
(SETQ STOP  '( A U G) )
(SETQ PS    '( (Phe (U U U) (U U C)) (Leu ( U U A) ( U U G)) (Llw (A U U) (A U C) (A U A)) ) )


(FUNC PROT (X) (WITH A B C) (SETQ (A B C) X))

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




