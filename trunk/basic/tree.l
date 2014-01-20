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
(setq STATE '( (arad (zerin 75) (sibiu 140) (timis 118))
(timis (logoj 111) (arad 118))
(zerin (arad 75) (oradca 71))
(sibiu (arad 140) (oradca 151) (fagaras 99) (rimni 80))
(lugoj (mecha 70) (timis 111))
(fagaras (sibiu 99) (bucharest 211))
(rimi (sibiu 80) (pitesti 97) (place 145))
(pitesti (rimi 97) (place 138) (bucharest 101))
(bucharest (urzi 85) (pitesti 101) (fagaras 211))
))

;initial arad - goal bucharest

(func GSEARCH (init goal) (PR init STATE))

(gsearch 'arad 'bucharest')




