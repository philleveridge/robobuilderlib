!do
(DEF DX (X W S) (- (/ (* S X) (- W 1)) (/ S 2)))

(DEF PT ()
(setq fn "test.jpg" s22 (getservo 22) s24 (getservo 24) vm 1 loop 1 filt 1 diff 0)

(while  loop
  (setq k (key 0))
  (cond (= k `q) (setq loop 0)
        (= k `a) (setq s22 (+ s22 2))
        (= k `s) (setq s22 (- s22 2))
        (= k `w) (setq s24 (+ s24 2))
        (= k `z) (setq s24 (- s24 2))
        (= k `v) (setq vm   (- 1 vm))
        (= k `f) (setq filt (- 1 filt))
        (= k `d) (setq diff (- 1 diff))
  )

  (pr  k "," s22 "," s24 "," filt "," vm)



  (if filt (setq I (image RAW fn)) (setq I (image filter fn '(120 175 40 70 30 40))))
  (setq IS (IMAGE SCALE I 32 32))  
  (if vm (IMAGE SHOW IS))

  (setq t (IMAGE MOMENT IS))
  (setq nx (dx (car t)  32 12))
  (setq ny (dx (cadr t) 32 -12))
  (pr x "," y ": " nx "," ny) 
  
  (setq s22 (+ s22 nx))
  (setq s24 (+ s24 ny))
  
  (setservo 22 s22)
  (setservo 24 s24)
  (WAIT 200)
)
(setservo 22 'P)
(setservo 24 'P)
(pr "Done"))

