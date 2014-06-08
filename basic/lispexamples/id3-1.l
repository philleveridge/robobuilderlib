!do
;decision tree based learning
;to be completed
 
(setq      Altern?      0 ) 
(setq      Bar?       	1 )
(setq      Fri?         2 )
(setq      Hungry?      3 )
(setq      Patron?      4 )
(setq      Price?       5 )
(setq      Rain?        6 )
(setq      Restr?    	7 )
(setq      Type?   	8 )
(setq      Est?        	9 )
(setq      Wait?        10)
 
(setq tdata '(
(Yes No  No  Yes  Some  $$$    No    Yes    French   0–10   Yes) 
(Yes No  No  Yes  Full  $      No    No     Thai     30–60  No )
(No  Yes No  No   Some  $      No    No     Burger   0–10   Yes) 
(Yes No  Yes Yes  Full  $      Yes   No     Thai     10–30  Yes)
(Yes No  Yes No   Full  $$$    No    Yes    French   >60    No )
(No  Yes No  Yes  Some  $$     Yes   Yes    Italian  0–10   Yes) 
(No  Yes No  No   None  $      Yes   No     Burger   0–10   No ) 
(No  No  No  Yes  Some  $$     Yes   Yes    Thai     0–10   Yes) 
(No  Yes Yes No   Full  $      Yes   No     Burger   >60    No )  
(Yes Yes Yes Yes  Full  $$$    No    Yes    Italian  10–30  No )
(No  No  No  No   None  $      No    No     Thai     0–10   No ) 
(Yes Yes Yes Yes  Full  $      No    No     Burger   30–60  Yes) 
))

(setq Alist '(Altern Bar Fri Hungry Patron Price Rain Restr Type Est))
 
 
(def B (q) (cond (= q 0.0) 0.0
	(= q 1.0) 0.0
	(- 0 (+ (* q (log2 q)) (* (- 1.0 q) (log2 (- 1.0 q)))))))
 
(def CountP (data)
	(setq cnt 0)
	(foreach i data
;              (pr (nth 10 i))
               (if (= (nth 10 i) "Yes") (inc 'cnt)))
;  	(pr "cnt=" cnt " len=" (len data))
	(or (= cnt 0) (= cnt (len data)))
)
 
(def TestP (X y e)
   (SETQ P 0.0 N 0.0)
   (FOREACH I e
               (IF (= (NTH X I) y) (if (= (nth 10 i) "Yes") (INC 'P) (INC 'N))))
;  (PR "P=" P " N=" N " B(P/P+N)= " (B (/ p (+ p n))))
   (* (/ (+ p n) (len e)) (B (/ p (+ p n))))
)
 
(def GetClass (x td)
       (setq lst '())
       (FOREACH I td
;             (pr "lst=" lst " ix=" (nth x i))
              (IF (NOT (MEMBER (NTH x I) LST)) (SETQ LST (CONS (NTH x I) LST)))
       )
       lst
)
 
(def findexamples (att val examples)
       (setq nl '())
       (foreach e examples (if (= (nth att e) val) (setq nl (cons e nl))))
       nl)

(def delatt (attlist att) 
	(setq lst '()) 
	(foreach a attlist 
;		(pr a "," att)
		(if (not (= a att)) (setq lst (cons a lst)))) 
	(rev lst))
 
; Gain(A) = B(p/p+n ) − Remainder (A)
(def IMPORTANCE (x td)
       (setq sm 0.0 lst (getclass x td))
       (foreach e lst
              (setq sm (+ sm (testp x e td)))
;             (pr  e " = " )
       )
       (- 1 sm)
)
 
(def PLURALITY-VALUE ( a )
; The function PLURALITY-VALUE selects the most common output
; value among a set of examples, breaking ties randomly
; (pr "pv a=" a)
	"yes")

 
; AI - modern approachfrom figure 18.5 -
;function DECISION-TREE-LEARNING (examples, attributes, parent examples) returns a tree
;  if examples is empty then return PLURALITY-VALUE(parent examples)
;  else if all examples have the same classification then return the classification
;  else if attributes is empty then return PLURALITY-VALUE(examples)
;  else
;    A ← argmaxa ∈ attributes IMPORTANCE(a, examples)
;    tree ← a new decision tree with root test A
;    for each value vk of A do
;        exs ← {e : e ∈ examples and e.A = vk }
;        subtree ← DECISION-TREE-LEARNING(exs, attributes − A, examples)
;        add a branch to tree with label (A = vk ) and subtree subtree
;    return tree
 
(def DECISION-TREE-LEARNING (examples attributes parent-examples)
	(with tree subtree ra mg)
	(if (null examples)   (return (PLURALITY-VALUE parent-examples)))
	(if (null attributes) (return (PLURALITY-VALUE examples)))
	(if (countp examples) (return (nth 10 (car examples))))

	(setq mg 0.0 ra 0)
	(foreach a attributes
		(setq g (IMPORTANCE a examples))		
		(if (> g mg) (setq mg g ra a))
        )
;       (pr (list ra mg))
	(setq tree (list "Att" (nth ra Alist) "=?"))
        (foreach vk (getclass ra examples)
		(setq nex (findexamples ra vk examples))
		(setq subtree (DECISION-TREE-LEARNING nex (delatt attributes ra) examples))
		(nconc tree (list ra "=" vk subtree) )
;		(pr "s=" subtree " tree=" tree)
        )
	tree
)
 
;testing
(pr (DECISION-TREE-LEARNING 
	tdata 
	(list Altern? Bar? Fri? Hungry? Patron? Price? Rain? Restr? Type? Est?) ) )
 
