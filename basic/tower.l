!DO

(
FUNC TOWER (N) (TRANSFER 'A 'B 'C N)
)


(
FUNC MOVEDISK (FROM TO) (LIST (LIST 'MOVE 'DISK 'FROM FROM 'TO TO))
)


(
FUNC TRANSFER (FROM TO SPARE NUMBER) 
     (COND ( (= NUMBER 1) (MOVEDISK FROM TO) 
             1 (APPEND (TRANSFER FROM SPARE TO (- NUMBER 1)) 
                       (MOVEDISK FROM TO) 
                       (TRANSFER SPARE TO FROM (- NUMBER 1)))
           ))
)


