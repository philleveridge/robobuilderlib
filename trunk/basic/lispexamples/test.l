IMAGE LOAD 8 "test.jpg"
SETQ IM (IMP "@" 8 8)
SUM IM
FOREACH X IM (SET S (+ S X))
MAPCAR (LAMBDA (X) (SETQ S (+ S X))) '(1 2 3)
MAPCAR (LAMBDA (X) (SETQ S (+ S X))) [5 6 7] 
/ [1 2 3] 6

FOR I '(0 7) (DO (FOR J '(0 7) (PR I "," J " : " (CELL IM I J))))

