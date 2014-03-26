!DO
 
(FUNC FUNCPROP (L)
(  FOREACH X L (SETQ E (CONS (LIST X 0) E)))
      (DICT E)
)
(FUNC PUTPROP (I V P) (SETK  I P V))
(FUNC REMPROP (I P)   (SETK  I P ))
(FUNC GETPROP (I P )  (GETK  I P ))
 
(FUNC PRPROP (i)  (PR (GETPROP i "COLOUR")))
 
(SETQ false 0 true 1 nil '() PLAN '() B1 0 B2 0 B3 0 B4 0 TABLE 0 HAND 0)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; basic facts about block world
 
(FUNC SETFACTS()
  (SETQ PLAN '())
  (SETQ BT    '(TYPE POSITION SIZE SUPPORTED-BY DIRECTLY-SUPPORTS COLOUR) )
  (SETQ B1    (FUNCPROP BT))
  (SETQ B2    (FUNCPROP BT))
  (SETQ B3    (FUNCPROP BT))
  (SETQ B4    (FUNCPROP BT))
  (SETQ TABLE (FUNCPROP '(DIRECTLY-SUPPORTS )))
  (SETQ HAND  (FUNCPROP '(GRASPING POSITION)))

  (PUTPROP 'B1 'BLOCK   'TYPE)
  (PUTPROP 'B1 '(1 1 0) 'POSITION)
  (PUTPROP 'B1 '(2 2 2) 'SIZE)
  (PUTPROP 'B1 'TABLE   'SUPPORTED-BY)
  (PUTPROP 'B1 '(B2)    'DIRECTLY-SUPPORTS)
  (PUTPROP 'B1 'RED     'COLOUR)
 
  (PUTPROP 'B2 'BLOCK   'TYPE)
  (PUTPROP 'B2 '(1 1 2) 'POSITION)
  (PUTPROP 'B2 '(2 2)   'SIZE)
  (PUTPROP 'B2 'B1      'SUPPORTED-BY)
  (PUTPROP 'B2 'BLUE    'COLOUR)
 
  (PUTPROP 'B3 'BLOCK   'TYPE)
  (PUTPROP 'B3 '(1 1 2) 'POSITION)
  (PUTPROP 'B3 '(2 2)   'SIZE)
  (PUTPROP 'B3 'TABLE   'SUPPORTED-BY)
  (PUTPROP 'B3 '(B4)    'DIRECTLY-SUPPORTS)
  (PUTPROP 'B3 'GREEN   'COLOUR)
 
  (PUTPROP 'B4 'PYRAMID 'TYPE)
  (PUTPROP 'B4 '(1 1 2) 'POSITION)
  (PUTPROP 'B4 '(2 2)   'SIZE)
  (PUTPROP 'B4 'B3      'SUPPORTED-BY)
  (PUTPROP 'B4 'BLUE    'COLOUR)
 
  (PUTPROP 'TABLE '(B1 B3)  'DIRECTLY-SUPPORTS)
 
  (PUTPROP 'HAND  nil      'GRASPING)
  (PUTPROP 'HAND  '(1 1 1) 'POSITION)
  (PR 'DONE)
)
 
(FUNC SHOWFACTS ()
  (WITH X)
  (FOREACH X '(B1 B2 B3 B4) (PR X ; (GETPROP X "COLOUR") ; (GETPROP X "TYPE") ; (GETPROP X "SUPPORTED-BY")))
  (PR 'TABLE ; (GETPROP 'TABLE "DIRECTLY-SUPPORTS"))
  (PR 'HAND  ; (GETPROP 'HAND  "GRASPING"))
  'DONE
) 
 
 
;;; simulated block world
;
;-----
;    |
;   HAND
;
;   ----      ^
;   |B2|     /B4\
;   ----     ----
;   ----     ----
;   |B1|     |B3|
;   ----     ----
; ==================
 
 
(FUNC PUTON (obj sup)
  (PR "DBG: PUTON" obj sup)
  (SETQ PLAN ())
  (WITH (place 0))
    (COND (SETQ place (FINDSPACE sup obj)) (PUTAT obj place)
          (SETQ place (MAKESPACE sup obj)) (PUTAT obj place)
    )
    (REV PLAN)
)
   
(FUNC PUTAT (obj pl)
     (PR "DBG: PUTAT" obj pl)
     (GRASP obj)
     (MOVEOBJ obj pl)
     (UNGRASP obj)
)
 
(FUNC GRASP (obj)
(PR "DBG: GRASP" obj)
(WITH (k 0))
  (IF (<> obj (GETPROP 'HAND 'GRASPING)) 
  (DO
    (COND    (GETPROP obj 'DIRECTLY-SUPPORTS)       (CLEARTOP obj)
             (SETQ k (GETPROP 'HAND 'GRASPING))     (GETPROP ridof k))
    (MOVEHAND (TOPCENTRE obj))
    (PUTPROP 'HAND obj 'GRASPING)
    (SETQ PLAN (CONS (LIST 'GRASP obj) PLAN))
  )
  )
)
 
(FUNC MOVEOBJ (obj ns)
(PR "DBG: MOVEOBJ" obj ns)
   (REMOVESUPPORT obj)
   (MOVEHAND (NEWTOPCENTRE obj ns))
   (PUTPROP obj ns 'POSITION)
   (ADDSUPPORT obj ns)
   (SETQ PLAN (CONS (LIST 'MOVEOBJ obj ns) PLAN))
)
 
(FUNC MOVEHAND (pos)
(PR "DBG: MOVEHAND" pos)
(PUTPROP 'HAND pos 'POSITION)
)
 
(FUNC UNGRASP (obj) 
(PR "DBG: UNGRASP " obj )
  (IF  (GETPROP obj 'SUPPORTED-BY)
       (DO (remprop 'HAND 'GRASPING)
           (SETQ PLAN (CONS (LIST 'UNGRASP obj) PLAN))
       )
       nil
  )
)
 
(FUNC GETRIDOF (obj)
(PR "DBG: getridoff" obj)
(PUTAT obj (FINDSPACE 'TABLE obj))
)
 
(FUNC CLEARTOP (obj)
(PR "DBG: CLEARTOP" obj)
  (map GETRIDOF (GETPROP obj 'DIRECTLY-SUPPORTS))
)
 
(FUNC REMOVESUPPORT (obj)
(PR "DBG: REMOVESUPPORT" obj)
   (SETQ sup (GETPROP obj 'SUPPORTED-BY))
(PR "DBG: REMOVESUPPORT - supported" sup)
   (IF sup (PUTPROP sup (REMPROP obj (GETPROP sup 'DIRECTLY-SUPPORTS)) 'DIRECTLY-SUPPORTS))
   (PUTPROP obj null 'SUPPORTED-BY)
)
 
(FUNC ADDSUPPORT (obj pl)
(PR "DBG: ADDSUPPORT" obj pl)
(SETQ sup (GETOBJUNDER pl))
(PR "DBG: ADDSUPPORT " sup " - " (GETPROP sup 'TYPE))
 
(IF (OR (= sup 'TABLE) (= (GETPROP sup 'TYPE) 'BOX) (= (GETPROP sup 'TYPE) 'BLOCK))
       (PUTPROP sup (CONS obj (GETPROP sup 'DIRECTLY-SUPPORTS)) 'DIRECTLY-SUPPORTS))
(PUTPROP obj sup 'SUPPORTED-BY)
)
 
(FUNC MAKESPACE (sup obj)
(PR "DBG: MAKESPACE" sup obj)
(WITH  (p 0) (n 0) (j 0) (fl true))
(SETQ j (GETPROP sup 'DIRECTLY-SUPPORTS))
(WHILE fl
(PR "DBG: MAKESPACE =" j)
   (SETQ n (CAR j))
   (SETQ j (CDR j))
   (GETRIDOF n)
   (IF (= p (FINDSPACE sup obj)) (SETQ fl false))
)
p
)

 
(FUNC FINDSPACE (sup obj)
    (PR "DBG: FINDSPACE" sup obj)
    (WITH k)
    (COND (= sup 'TABLE)          (LIST 'space 'above sup 'for obj)
          (SETQ k (GETPROP sup 'DIRECTLY-SUPPORTS))
             ( DO
              (PR sup " supports " k)
              (PR "type 't' IF FINDSPACE wins")
              (IF (NOT (= (KEY 1) "T")) nil (LIST 'space 'above sup 'for obj))
             )
          (LIST 'space 'above sup 'for obj)
    )
)
 
(FUNC GETOBJUNDER (pl)
(PR "DBG: GETOBJUNDER" pl)
(CAR (CDR (CDR pl))))
 
(FUNC TOPCENTRE (obj) 
(PR "DBG: TOPCENTRE" obj)
(LIST 'TOPCENTRE obj))
 
(FUNC NEWTOPCENTRE (obj pl)
(PR "DBG: NEWTOPCENTRE" obj pl)
(LIST 'NEWTOPCENTRE obj pl))
 
(SETFACTS)
 
(PR "Blockworld! - initial state")
(PR "")
(SHOWFACTS)
 
(PR "")
(PR "Commands : (SETFACTS)      - reset facts")
(PR "           (SHOWFACTS)     - show facts")
(PR "           (PUTON 'B1 'B3) - put 'B1 on 'B3 and show PLAN")
'Ready


