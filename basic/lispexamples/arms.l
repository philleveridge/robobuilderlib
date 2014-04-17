!do
;LOAD "lispexamples/arms.l"

(def lefty(x)  (- ( + (- (/ (* x (* x x)) 20000)  (/ (* x x) 47) ) (* 3 x)) 46))
(def leftz(x)  (- ( + (- (/ (* x (* x x)) 12500)  (/ (* x x) 29) ) (* 5 x)) 108))

(def righty(x) (+ ( + (- (/ (* x (* x x)) 20000)  (/ (* x x) 57) ) (* 2 x)) 87))
(def rightz(x) (+ ( + (- (/ (* x (* x x)) 12500)  (/ (* x x) 37) ) (* 3 x)) 25))


;(FOR X '(50 190) (PR X ; (LEFTY X) ; (LEFTZ X) ; (RIGHTY X) ; (RIGHTZ X)))

;(SETQ m [ 1 1 0 0 ; 0 0 0 0 ; 1 0 0 0 ; 0 0 0 1])
;(SETQ I (IMAGE MAT M))
;(MAT 'i 2 1)
;(IMAGE LOAD 4)
;(IMAGE SERIAL 4)
;(IMAGE NORM 4)

(SETQ WE [ 1  1  0  0
           1  1  0  0 
           0  0 -1 -1 
           0  0 -1 -1  0 ; 

           0  0  1  1 
           0  0  1  1
          -1 -1  0  0
          -1 -1  0  0  0 ;
           0  0 -1 -1 
           0  0 -1 -1 
           1  1  0  0
           1  1  0  0  0 ;
          -1 -1  0  0
          -1 -1  0  0
           0  0  1  1 
           0  0  1  1  0 ;
           0  1  1  0 
           0  1  1  0 
           0 -1 -1  0
           0 -1 -1  0-1  0 ;
           0  0  0  0 
          -1 -1  1  1 
          -1 -1  1  1 
           0  0  0  0  0 ;
           0 -1 -1  0 
           0 -1 -1  0 
           0  1  1  0 
           0  1  1  0  0 ;
           0  0  0  0
           1  1 -1 -1 
           1  1 -1 -1 
           0  0  0  0  0 ])

(Pr "Loop - press 'Q' to stop")

(SETQ X 0)
(SETQ Y 0)

(WHILE (<> (KEY 1) 81)
     (Pr 'loop ; X ; Y)
     (SETQ IP (+ (TRN [0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1]) (IMAGE SERIAL 4)))
     (SETQ O  (SIG (* WE IP)))
     ;(pr O)
     (if (> (cell o 0 0) 0.5) (DO (PR "DR") (+ X 1) (+ Y 1)))
     (if (> (cell o 1 0) 0.5) (DO (PR "DL") (- X 1) (+ Y 1)))
     (if (> (cell o 2 0) 0.5) (DO (PR "UR") (+ X 1) (- Y 1)))
     (if (> (cell o 3 0) 0.5) (DO (PR "UL") (- X 1) (- Y 1)))

     (if (> (cell o 4 0) 0.5) (DO (PR "D")  (+ Y 1)))
     (if (> (cell o 5 0) 0.5) (DO (PR "L")  (- X 1)))
     (if (> (cell o 6 0) 0.5) (DO (PR "U")  (- Y 1)))
     (if (> (cell o 7 0) 0.5) (DO (PR "R")  (+ X 1)))
)
