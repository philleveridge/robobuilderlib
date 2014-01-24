!DO 
;function graphsearch (problem)
;frontier = {initial); explore ={}
;loop:
;if frontier is empty return FAIL
;path = remove_choice (frontier)
;s = path.end ; add  s to explored
;if s = goal return path
;for a in actions
;  add [path +a -> result (s,a)] to frontier
;  unless result(s,a) in frontier + explored

;state space
(setq STATE '( 
	(TIMIS     (LOGOJ 111) (ARAD 118))
	(ZERIN     (ARAD 75)   (ORADCA 71))
	(LUGO      (MECHA 70)  (TIMIS 111))
	(FAGARAS   (SIBIU 99)  (BUCHAREST 211))
	(ARAD      (ZERIN 75)  (SIBIU 140)   (TIMIS 118))
	(RIM       (SIBIU 80)  (PITESTI 97)  (PLACE 145))
	(PITESTI   (RIMI 97)   (PLACE 138)   (BUCHAREST 101))
	(BUCHAREST (URZI 85)   (PITESTI 101) (FAGARAS 211))
	(SIBIU     (ARAD 140)  (ORADCA 151)  (FAGARAS 99) (RIMNI 80))
))

;initial arad - goal bucharest

(func GPATH (x) (FOREACH y STATE (IF (= (CAR y) x) (BREAK (PR x " .. " (CDR y)))) ))


(func GSEARCH (init goal) 
   (PR "starting from .. " init " going to " goal)
   (setq FRONTIER (LIST init) EXPLORED '())

   (WHILE (NOT (NULL FRONTIER))
       (SETQ top (CAR FRONTIER))
       (SETQ action (GPATH top)) 
       (FOREACH a action (PR a))
       (setq FRONTIER (CDR FRONTIER))
   )
)

(GSEARCH 'ARAD 'BUCHAREST')




