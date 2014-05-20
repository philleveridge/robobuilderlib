!do

(SETQ M  '() )

(setq LA '(10 11 12))
(setq RA '(13 14 15))

(def BEST (T)
  (SETQ R  (FIND T M))
  (PR (NTH (- (LEN M) (LEN (MEMBER (MIN R) R))) M))
)
 
(def FIND (X Y) (COND (NULL Y) '() 
                       (CONS (CMP X (CAR Y)) (FIND X (CDR Y))) 
                 ))
  
(def CMP (x Y) 
  (COND  (NULL X) 0
         (ATOM X) (ABS (- X Y))
         (+ (CMP (CAR X) (CAR Y)) (CMP (CDR X) (CDR Y)))
))


(def GS (x) 
    (if (null x) '() 
        (cons (getservo (car x)) (gs (cdr x))))
)

(def store (X) (setq m (cons (gs x) m)))


(DEF FOO    (X)   (+ 127 (* -1 (- X 127)))  )
(DEF MODULO (X Y) (IF (>= X 0) (MOD X Y) (+ Y (MOD X Y))))

(def bound (A)
        (setq lb 254 ub 0 t 0)
        (setservo A 'P)
        (while (< (key) 0)
               (setq t  (getservo a))
               (setq lb (min lb t))
               (setq ub (max ub t))
               (pr "ub=" ub ", lb=" lb)
               (wait 500)
        ))

(def mem ()
       (while (< (key) 0)
             (pr ">")
             (wait 1000)
             (store LA)
       )
)

(SETQ Z '(120 125 129 130))
(pr (MAPCAR FOO Z))


