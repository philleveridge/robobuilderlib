!do
;from book AI : structures and strategies by G F Luger
; function unify(E1, E2);
; begin
;   case
;    both E1 and E2 are constants or the empty list:    % recursion stops
;      if E1 = E2 then return {}
;      else return FAIL;
;    E1 is a variable:
;      if E1 occurs in E2 then return FAIL
;      else return {E2/E1};
;    E2 is a variable:
;      if E2 occurs in E1 then return FAIL
;      else return {E1/E2}
;    either E1 or E2 are empty then return FAIL         % the lists are of different sizes
;    otherwise:                                         % both E1 and E2 are lists
;    begin
;       HE1   := first element of E1;
;       HE2   := first element of E2;
;       SUBS1 := unify (HE1,HE2);
;       if SUBS1 : = FAIL then return FAIL;
;       TE1   := apply (SUBS1, rest of E1);
;       TE2   := apply (SUBS1, rest of E2);
;       SUBS2 := unify (TE1, TE2);
;       if SUBS2 = FAIL then return FAIL;
;       else return composition(SUBS1,SUBS2)
;    end
;   end 
; end

(setq NIL    '())
(def  varp   (x)   (and (atom x) (=  (lft x 1) '?)))
(def  constp (x)   (and (atom x) (<> (lft x 1) '?)))
(def  comp   (x y) (cond (null x) y (null y) x (list x y)))

(def  subst (x y z) 
	(cond 	(null z) 	NIL
		(or (null x) (null y)) z			 
		(atom z) 	(if (= z x) y z) 
		(cons (subst x y (car z)) (subst x y (cdr z))))) 

(def unify (e1 e2) 
	(with he1 he2 subs1 te1 te2 subs2)
	(pr "Unify " e1 " <-> " e2)
	(cond 
		(and (null e1) (null e2))   	NIL
		(and (constp e1) (constp e2))   (if (= e1 e2)      NIL   'FAIL)
		(varp e1)                       (if (member e1 e2) 'FAIL (list e1 e2))
		(varp e2)                       (if (member e2 e1) 'FAIL (list e2 e1))
		(or (null e1) (null e2))        'FAIL
;otherwise:
		(do 
		  (setq he1   (car e1))
		  (setq he2   (car e2))
		  (setq subs1 (unify he1 he2))
		  (if (= subs1 'FAIL) (return 'FAIL))

		  (setq te1   (subst (car subs1) (cadr subs1) (cdr e1)))
		  (setq te2   (subst (car subs1) (cadr subs1) (cdr e2)))	 
		  (setq subs2 (unify te1 te2))
		  (if (= subs2 'FAIL) (return 'FAIL))
		  (comp subs1 subs2)
		)
	)
)

(pr "Test 1\n" (unify '(?x ?y) '(a b)))
(pr "Test 2\n" (unify '(?x ?x) '(a b)))
(pr "Test 3\n" (unify '(parents ?x (father ?x) (mother bill)) '(parents bill (father bill) ?y)))


