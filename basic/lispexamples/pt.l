!do
(DEF DX (X W S) (- (/ (* S X) (- W 1)) (/ S 2)))

(DEF PT ()
(setq s22 (getservo 22))
(setq s24 (getservo 24))
(setq vm 1 loop 1)
(while  loop
  (setq k (key))
  (if (= k `q) (setq loop 0))
  (if (= k `a) (setq s22 (+ s22 2)))
  (if (= k `s) (setq s22 (- s22 2)))
  (if (= k `w) (setq s24 (+ s24 2)))
  (if (= k `z) (setq s24 (- s24 2)))
  (if (= k `v) (setq vm  (- 1   vm)))

  (setq I (image RAW "test.jpg"))
  (setq IS (IMAGE SCALE I 32 32))  
  (if vm (IMAGE SHOW IS))

  (setq (x y) (IMAGE MOMENT IS))
  (setq nx (dx x 32 4))
  (setq ny (dx y 32 5))
  (pr x "," y ": " nx "," ny) 
  
  (setq s22 (+ s22 nx))
  (setq s24 (+ s24 ny))
  
  (pr  k "," s22 "," s24)
  (setservo 22 s22)
  (setservo 24 s24)
  (image WAIT 200)
)
(setservo 22 'P)
(setservo 24 'P)
(pr "Done"))

