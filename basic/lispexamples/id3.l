!do

; employee (name; address; serial-number; department; salary)
(def make-employee (name address sn dept salary) (list name address sn dept salary))
(def name-employee    (rec)  (nth 0 rec))
(def address-employee (rec)  (nth 1 rec))
(def sn-employee      (rec)  (nth 2 rec))
(def dept-employee    (rec)  (nth 3 rec))
(def salary-employee  (rec)  (nth 4 rec))

(setq TDATA '(
;(	risk 	history debt 	collateral 	income)
	(high 	bad 	high 	none 		$0-$15k)
	(high 	unknown high 	none 		$15k-$35k)
	(moderate unknown low 	none 		$15k-$35k)
	(high 	unknown low 	none 		$0-$15k)
	(low 	unknown low 	none 		>$35k)
	(low 	unknown low 	adequate 	>$35k)
	(high 	bad 	low 	none 		$0-$15k)
	(moderate bad 	low 	adequate 	>$35k)
	(low 	good 	low 	none 		>$35k)
	(low 	good 	high 	adequate 	>$35k)
	(high 	good 	high 	none 		$0-$15k)
	(moderate good	high 	none 		$15k-$35k)
	(low 	good 	high 	none 		>$35k)
	(high 	bad 	high 	none 		$15k-$35k)))

(def risk-data    	(rec)  (nth 0 rec))
(def history-data 	(rec)  (nth 1 rec))
(def debt-data      	(rec)  (nth 2 rec))
(def collateral-data    (rec)  (nth 3 rec))
(def income-data  	(rec)  (nth 4 rec))

;(partition : test-name  test components info-gain)

(SETQ NEWEMP (MAKE-EMPLOYEE '(JANE DOE) "123 main st" 9876 'Sales 4500.00))

(pr "name = " (name-employee newemp))
(foreach r TDATA (pr (risk-data r) "," (income-data r)))



