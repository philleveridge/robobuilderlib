!do

(def lefty(x)  (- ( + (- (/ (* x (* x x)) 20000)  (/ (* x x) 47) ) (* 3 x)) 46))
(def leftz(x)  (- ( + (- (/ (* x (* x x)) 12500)  (/ (* x x) 29) ) (* 5 x)) 108))

(def righty(x) (+ ( + (- (/ (* x (* x x)) 20000)  (/ (* x x) 57) ) (* 2 x)) 87))
(def rightz(x) (+ ( + (- (/ (* x (* x x)) 12500)  (/ (* x x) 37) ) (* 3 x)) 25))


(FOR X '(50 190) (PR X ; (LEFTY X) ; (LEFTZ X) ; (RIGHTY X) ; (RIGHTZ X)))




