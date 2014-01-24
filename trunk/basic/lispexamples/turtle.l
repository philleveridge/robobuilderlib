!DO

;# turn, and add randomness to the turning command
;orientation = self.orientation + float(turn) + random.gauss(0.0, self.turn_noise)
;orientation %= 2 * pi

;# move, and add randomness to the motion command
;dist = float(forward) + random.gauss(0.0, self.forward_noise)
;x = self.x + (cos(orientation) * dist)
;y = self.y + (sin(orientation) * dist)
;x %= world_size    # cyclic truncate
;y %= world_size

;# set particle
;res = robot()
;res.set(x, y, orientation)
;res.set_noise(self.forward_noise, self.turn_noise, self.sense_noise)


(FUNC ROUND (X) (INT (+ 0.5 X)))
(setq LMS '( (2 2) (4 4) ))

(def CHT  (X Y T) (AND (= X (ROUND (CAR T))) (= Y (ROUND (CAR (CDR T))))))
(def CHL  (X Y L) (AND (= X (CAR L)) (= Y (CAR (CDR L)))))
(def MOVE (T A D) (WITH X Y R) (SETQ (X Y R) T ) (LIST ( + X (* D (COS A))) (+ Y (* D (SIN A))) (+ A R) ))
(def PG   ()      (FOR X '(1 10) (FOR Y '(1 10) (IF (CHT X Y TURTLE) (PR 'x ;) (PR '_ ;))) (PR)))

(setq TURTLE '(1 1 0.1))
(PG)
(PR 'ok)
(setq TURTLE '(2 3 0.2))
(PG)
(PR 'ok)
(SETQ TURTLE (MOVE TURTLE 0.1 4))
(PG)
(PR TURTLE)






